#ifndef FLEETPROVISIONINGSERVICE_H
#define FLEETPROVISIONINGSERVICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <mutex>

#include "esp_err.h"
#include "sdkconfig.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "StandardDefines.h"
#include "server/IDeviceService.h"
#include "logger/ILogger.h"

#include "IFleetProvisioningService.h"

/* @Component */
class FleetProvisioningService : public IFleetProvisioningService {
    
    Public FleetProvisioningService()
        : mqttClient(nullptr),
          mqttStarted(false),
          enrollmentStarted(false),
          pendingSubscriptions(0),
          credentialSavePending(false),
          status(EnrollmentStatus::NotStarted) {
            fpProfile = deviceService->GetFleetProvisioningProfile();
    }

    Public Bool IsEnrolled() override {
        return deviceService->GetDeviceIdentityProfile().has_value();
    }

    Public Void EnrollDevice() override {
        if(IsEnrolled()) {
            logger->Info(Tag::Untagged, "Device already enrolled");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::AlreadyEnrolled;
            return;
        }
        
        logger->Info(Tag::Untagged,
                     "Starting enrollment for serial=" +
                     deviceService->GetSerialNumber());

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::InProgress;
            credentialSavePending = false;
        }

        fpProfile = deviceService->GetFleetProvisioningProfile();
        CloseConnection();
        StartMqttClient();
    }

    Public EnrollmentStatus GetEnrollmentStatus() override {
        std::lock_guard<std::mutex> lock(mutex_);
        return status;
    }

    Public EnrollmentStatus WaitForEnrollment(Int timeoutMs) override {
        Const Int step = 200;
        Int waited = 0;
        while (waited < timeoutMs) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (status != EnrollmentStatus::NotStarted &&
                    status != EnrollmentStatus::InProgress) {
                    return status;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(step));
            waited += step;
        }
        return GetEnrollmentStatus();
    }

    Public Void CloseConnection() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (mqttClient) {
            logger->Info(Tag::Untagged, "Closing MQTT connection...");
            esp_mqtt_client_stop(mqttClient);
            esp_mqtt_client_destroy(mqttClient);
            mqttClient = nullptr;
            mqttStarted = false;
        }
    }

    /* @Autowired */
    Private IDeviceServicePtr deviceService;

    /* @Autowired */
    Private ILoggerPtr logger;

    FleetProvisioningProfileData fpProfile;

    Private StdString enrollmentBrokerUri;
    Private StdString enrollmentBrokerHost;
    Private StdString enrollmentClientId;

    static constexpr Int kEnrollmentNetworkTimeoutMs = 30000;

    Private esp_mqtt_client_handle_t mqttClient;
    Private Bool mqttStarted;
    Private Bool enrollmentStarted;
    Private Int pendingSubscriptions;
    Private Bool credentialSavePending;
    Private std::mutex mutex_;
    Private EnrollmentStatus status;

    // Buffers for received credentials
    Private StdString devicePrivateKeyPem;
    Private StdString awsDeviceCertPem;
    Private StdString awsCaCertificatePem;
    Private StdString ownershipToken;
    Private StdString thingName;
    Private StdString pendingTenantId;

    // Parsing buffers (heap-backed via singleton; not on mqtt_task stack)
    Private Char ownershipBuf[768];
    Private Char certIdBuf[128];
    Private Char keyEscBuf[4096];
    Private Char certEscBuf[4096];
    Private Char keyPemBuf[4096];
    Private Char certPemBuf[4096];

    Private Static Void EnrollmentCompleteTask(Void* arg) {
        Var self = static_cast<FleetProvisioningService*>(arg);
        const StdString tenantId = self->pendingTenantId;
        self->CloseConnection();
        vTaskDelay(pdMS_TO_TICKS(500));
        self->SaveReceivedCredentials(tenantId);
        vTaskDelete(nullptr);
    }

    /** After provision accepted: save on enroll_save (closes MQTT off mqtt_task). */
    Private Void ScheduleCredentialSave() {
        if (xTaskCreate(EnrollmentCompleteTask, "enroll_save", 12288, this, 8, nullptr) != pdPASS) {
            std::lock_guard<std::mutex> lock(mutex_);
            credentialSavePending = false;
            status = EnrollmentStatus::Failed_Provision;
            logger->Error(Tag::Untagged, "Failed to schedule credential save task");
        }
    }

    Private bool json_extract_string(const char *json, const char *key, char *out, size_t out_len) {
        char pattern[64];
        snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
        const char *start = strstr(json, pattern);
        if (!start) {
            return false;
        }
        start += strlen(pattern);
        const char *end = strchr(start, '"');
        if (!end || static_cast<size_t>(end - start) >= out_len) {
            return false;
        }
        memcpy(out, start, static_cast<size_t>(end - start));
        out[end - start] = '\0';
        return true;
    }
    
    Private void unescape_json_to_pem(const char *escaped, char *out, size_t out_len) {
        size_t j = 0;
        for (size_t i = 0; escaped[i] != '\0' && j + 1 < out_len; ++i) {
            if (escaped[i] == '\\' && escaped[i + 1] == 'n') {
                out[j++] = '\n';
                ++i;
            } else if (escaped[i] == '\\' && escaped[i + 1] == '\\') {
                out[j++] = '\\';
                ++i;
            } else {
                out[j++] = escaped[i];
            }
        }
        out[j] = '\0';
    }

    /** Ensures mqtts URI includes explicit port (ESP-TLS requires it for AWS IoT). */
    Private Static StdString NormalizeMqttUri(const StdString& endpoint) {
        StdString uri = endpoint;
        if (uri.find("://") == StdString::npos) {
            return "mqtts://" + uri + ":8883";
        }
        const size_t hostStart = uri.find("://") + 3;
        const size_t slash = uri.find('/', hostStart);
        const size_t colon = uri.find(':', hostStart);
        const Bool hasPort = (colon != StdString::npos && (slash == StdString::npos || colon < slash));
        if (!hasPort) {
            if (slash != StdString::npos) {
                uri.insert(slash, ":8883");
            } else {
                uri += ":8883";
            }
        }
        return uri;
    }

    Private Static Void ParseBrokerHostPort(const StdString& uri, StdString& host, Int& port) {
        port = 8883;
        host.clear();
        const size_t scheme = uri.find("://");
        const size_t hostStart = (scheme == StdString::npos) ? 0 : scheme + 3;
        const size_t colon = uri.find(':', hostStart);
        const size_t slash = uri.find('/', hostStart);
        if (colon != StdString::npos && (slash == StdString::npos || colon < slash)) {
            host = uri.substr(hostStart, colon - hostStart);
            port = atoi(uri.c_str() + colon + 1);
        } else if (slash != StdString::npos) {
            host = uri.substr(hostStart, slash - hostStart);
        } else {
            host = uri.substr(hostStart);
        }
    }

    Private Void StartMqttClient() {
        if (mqttStarted) {
            return;
        }
        mqttStarted = true;
        enrollmentStarted = false;
        pendingSubscriptions = 0;

        enrollmentBrokerUri = NormalizeMqttUri(fpProfile.mqttEndpoint);
        Int brokerPort = 8883;
        ParseBrokerHostPort(enrollmentBrokerUri, enrollmentBrokerHost, brokerPort);
        enrollmentClientId = deviceService->GetSerialNumber();

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.broker.address.hostname = enrollmentBrokerHost.c_str();
        mqtt_cfg.broker.address.port = brokerPort;
        mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_SSL;
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
        mqtt_cfg.broker.verification.crt_bundle_attach = esp_crt_bundle_attach;
        mqtt_cfg.broker.verification.certificate = nullptr;
#else
        mqtt_cfg.broker.verification.certificate = fpProfile.caCertificatePem.c_str();
        mqtt_cfg.broker.verification.certificate_len = fpProfile.caCertificatePem.size() + 1;
#endif
        mqtt_cfg.credentials.client_id = enrollmentClientId.c_str();
        mqtt_cfg.credentials.authentication.certificate = fpProfile.clientCertificatePem.c_str();
        mqtt_cfg.credentials.authentication.certificate_len =
            fpProfile.clientCertificatePem.size() + 1;
        mqtt_cfg.credentials.authentication.key = fpProfile.clientPrivateKeyPem.c_str();
        mqtt_cfg.credentials.authentication.key_len = fpProfile.clientPrivateKeyPem.size() + 1;
        mqtt_cfg.network.disable_auto_reconnect = true;
        mqtt_cfg.network.timeout_ms = kEnrollmentNetworkTimeoutMs;
        mqtt_cfg.buffer.size = 4096;
        mqtt_cfg.buffer.out_size = 4096;
        mqtt_cfg.task.stack_size = 20480;

        // Allow DNS/route to AWS IoT to settle (internet check does not validate *.amazonaws.com).
        vTaskDelay(pdMS_TO_TICKS(2000));

        mqttClient = esp_mqtt_client_init(&mqtt_cfg);
        if (!mqttClient) {
            logger->Error(Tag::Untagged,
                "Enrollment MQTT init failed host=" + enrollmentBrokerHost);
            mqttStarted = false;
            return;
        }
        esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_ANY, MqttEventHandler, this);
        const esp_err_t startErr = esp_mqtt_client_start(mqttClient);
        if (startErr != ESP_OK) {
            logger->Error(Tag::Untagged,
                "Enrollment MQTT start failed host=" + enrollmentBrokerHost);
            esp_mqtt_client_destroy(mqttClient);
            mqttClient = nullptr;
            mqttStarted = false;
            return;
        }

        logger->Info(Tag::Untagged,
            "MQTT client started for enrollment host=" + enrollmentBrokerHost
            + " port=" + std::to_string(brokerPort)
            + " client_id=" + enrollmentClientId
            + " timeout_ms=" + std::to_string(kEnrollmentNetworkTimeoutMs));
    }

    Private Static Void LogEnrollmentMqttError(FleetProvisioningService* self,
                                               esp_mqtt_event_handle_t event) {
        if (!event || !event->error_handle) {
            self->logger->Error(Tag::Untagged, "MQTT error during enrollment (no error_handle)");
            return;
        }
        const esp_mqtt_error_codes_t* err = event->error_handle;
        StdString line = "[Enrollment] MQTT transport error type="
            + std::to_string(static_cast<Int>(err->error_type))
            + " esp_err=" + std::to_string(static_cast<Int>(err->esp_tls_last_esp_err))
            + " mbedtls=" + std::to_string(err->esp_tls_stack_err)
            + " cert_flags=0x" + std::to_string(err->esp_tls_cert_verify_flags)
            + " sock_errno=" + std::to_string(err->esp_transport_sock_errno);
        if (err->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            line += " connect_return_code="
                + std::to_string(static_cast<Int>(err->connect_return_code));
        }
        self->logger->Error(Tag::Untagged, line);
    }

    Private Void SubscribeEnrollmentTopics() {
        pendingSubscriptions = 4;
        esp_mqtt_client_subscribe(mqttClient, fpProfile.createKeysAcceptedTopic.c_str(), 1);
        esp_mqtt_client_subscribe(mqttClient, fpProfile.createKeysRejectedTopic.c_str(), 1);
        esp_mqtt_client_subscribe(mqttClient, fpProfile.provisionAcceptedTopic.c_str(), 1);
        esp_mqtt_client_subscribe(mqttClient, fpProfile.provisionRejectedTopic.c_str(), 1);
        logger->Info(Tag::Untagged, "Subscribed to fleet provisioning topics");
    }

    Private Void PublishCreateKeysAndCertificate() {
        logger->Info(Tag::Untagged, "Publishing CreateKeysAndCertificate...");
        esp_mqtt_client_publish(mqttClient, fpProfile.createKeysRequestTopic.c_str(), "{}", 2, 1, 0);
    }

    Private Void HandleCreateKeysAccepted(CChar* payload, Int len) {
        std::lock_guard<std::mutex> lock(mutex_);
        logger->Info(Tag::Untagged, "CreateKeysAndCertificate ACCEPTED");

        StdString payloadStr(payload, len);

        if (!json_extract_string(payloadStr.c_str(), "certificateOwnershipToken", ownershipBuf, sizeof(ownershipBuf))) {
            logger->Error(Tag::Untagged, "Missing certificateOwnershipToken");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificateId", certIdBuf, sizeof(certIdBuf))) {
            logger->Error(Tag::Untagged, "Missing certificateId");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "privateKey", keyEscBuf, sizeof(keyEscBuf))) {
            logger->Error(Tag::Untagged, "Missing privateKey");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificatePem", certEscBuf, sizeof(certEscBuf))) {
            logger->Error(Tag::Untagged, "Missing certificatePem");
            return;
        }

        unescape_json_to_pem(keyEscBuf, keyPemBuf, sizeof(keyPemBuf));
        unescape_json_to_pem(certEscBuf, certPemBuf, sizeof(certPemBuf));

        ownershipToken      = ownershipBuf;
        devicePrivateKeyPem = keyPemBuf;
        awsDeviceCertPem    = certPemBuf;

        PublishProvisionRequest();
    }

    Private Void PublishProvisionRequest() {
        StdString json = "{\"certificateOwnershipToken\":\"" + ownershipToken +
                         "\",\"parameters\":{\"SerialNumber\":\"" +
                         deviceService->GetSerialNumber() + "\"}}";
        logger->Info(Tag::Untagged, "Publishing RegisterThing for serial=" +
                                    deviceService->GetSerialNumber());
        esp_mqtt_client_publish(mqttClient, fpProfile.provisionRequestTopic.c_str(),
                                json.c_str(), static_cast<Int>(json.size()), 1, 0);
    }

    Private Void HandleProvisionAccepted(CChar* payload, Int len) {
        StdString payloadStr(payload, len);

        Char thingNameBuf[128];
        if (!json_extract_string(payloadStr.c_str(), "thingName", thingNameBuf, sizeof(thingNameBuf))) {
            logger->Error(Tag::Untagged, "Missing thingName in provision response");
            return;
        }

        Char tenantIdBuf[128];
        if (!json_extract_string(payloadStr.c_str(), "tenantId", tenantIdBuf, sizeof(tenantIdBuf))) {
            logger->Error(Tag::Untagged, "Missing tenantId in provision response");
            return;
        }

        logger->Info(Tag::Untagged,
                     "Enrollment succeeded thing=" + StdString(thingNameBuf)
                     + " tenant=" + StdString(tenantIdBuf));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pendingTenantId = StdString(tenantIdBuf);
            thingName = StdString(thingNameBuf);
            credentialSavePending = true;
        }

        // Close enrollment MQTT before further TLS writes; save SPIFFS off mqtt_task.
        ScheduleCredentialSave();
    }

    Private Void SaveReceivedCredentials(const StdString& tenantId) {
        logger->Info(Tag::Untagged, "Saving received credentials for tenantId: " + tenantId);

        DeviceIdentityProfileDto identityDto;
        identityDto.clientCertificatePem = Optional<StdString>(std::move(awsDeviceCertPem));
        identityDto.clientPrivateKeyPem  = Optional<StdString>(std::move(devicePrivateKeyPem));
        identityDto.thingName = Optional<StdString>(std::move(thingName));
        identityDto.tenantId = Optional<StdString>(tenantId);
        ownershipToken.clear();
        awsCaCertificatePem.clear();

        deviceService->SetDeviceIdentityProfile(identityDto);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Success;
            credentialSavePending = false;
        }

        logger->Info(Tag::Untagged,
                     "Saved device identity profile for serial=" +
                     deviceService->GetSerialNumber());
    }

    Private Static Void MqttEventHandler(Void* handler_args, esp_event_base_t base,
                                         Int32 event_id, Void* event_data) {
        (void)base;
        (void)event_id;
        Var self = static_cast<FleetProvisioningService*>(handler_args);
        Var event = static_cast<esp_mqtt_event_handle_t>(event_data);
        const esp_mqtt_event_id_t ev = event
            ? event->event_id
            : static_cast<esp_mqtt_event_id_t>(event_id);

        switch (ev) {
            case MQTT_EVENT_BEFORE_CONNECT:
                break;

            case MQTT_EVENT_CONNECTED:
                self->logger->Info(Tag::Untagged, "MQTT connected");
                self->SubscribeEnrollmentTopics();
                break;

            case MQTT_EVENT_SUBSCRIBED:
                if (--self->pendingSubscriptions == 0 && !self->enrollmentStarted) {
                    self->enrollmentStarted = true;
                    self->PublishCreateKeysAndCertificate();
                }
                break;

            case MQTT_EVENT_DATA: {
                StdString topic(event->topic, event->topic_len);

                if (topic == self->fpProfile.createKeysAcceptedTopic) {
                    self->HandleCreateKeysAccepted(event->data, event->data_len);
                } else if (topic == self->fpProfile.provisionAcceptedTopic) {
                    self->HandleProvisionAccepted(event->data, event->data_len);
                } else if (topic == self->fpProfile.provisionRejectedTopic) {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    self->status = EnrollmentStatus::Failed_Provision;
                    self->logger->Error(Tag::Untagged, "Provisioning rejected");
                }
                break;
            }

            case MQTT_EVENT_ERROR: {
                Bool ignoreTeardownError = false;
                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    ignoreTeardownError = self->credentialSavePending;
                }
                if (ignoreTeardownError) {
                    break;
                }
                LogEnrollmentMqttError(self, event);
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->status = EnrollmentStatus::Failed_MqttConnect;
                self->mqttStarted = false;
                break;
            }

            case MQTT_EVENT_DISCONNECTED: {
                Bool ignoreTeardownError = false;
                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    ignoreTeardownError = self->credentialSavePending;
                }
                if (ignoreTeardownError) {
                    break;
                }
                std::lock_guard<std::mutex> lock(self->mutex_);
                if (self->status == EnrollmentStatus::InProgress) {
                    self->status = EnrollmentStatus::Failed_MqttConnect;
                    self->logger->Error(Tag::Untagged, "MQTT disconnected during enrollment");
                }
                break;
            }

            default:
                break;
        }
    }
};
#endif // FLEETPROVISIONINGSERVICE_H
