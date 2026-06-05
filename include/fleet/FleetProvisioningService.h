#ifndef FLEETPROVISIONINGSERVICE_H
#define FLEETPROVISIONINGSERVICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <mutex>

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
          awaitingMqttClose(false),
          mqttConnectionClosed(false),
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

        if (!HasSufficientHeapForEnrollment()) {
            LogInsufficientHeapForEnrollment();
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_InsufficientHeap;
            return;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::InProgress;
        }

        fpProfile = deviceService->GetFleetProvisioningProfile();
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


    Private esp_mqtt_client_handle_t mqttClient;
    Private Bool mqttStarted;
    Private Bool enrollmentStarted;
    Private Int pendingSubscriptions;
    Private Bool awaitingMqttClose;
    Private Bool mqttConnectionClosed;
    Private std::mutex mutex_;
    Private EnrollmentStatus status;

    static constexpr size_t kCredentialSaveHeapOverheadBytes = 16384;
    static constexpr size_t kEnrollmentHeapRequiredBytes = 24576;
    static constexpr Int kMqttCloseWaitStepMs = 50;
    static constexpr Int kMqttCloseWaitTimeoutMs = 5000;

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
        StdString tenantId;
        {
            std::lock_guard<std::mutex> lock(self->mutex_);
            tenantId = self->pendingTenantId;
            self->awaitingMqttClose = true;
            self->mqttConnectionClosed = false;
        }

        self->LogHeapStatus("Before MQTT close");
        self->CloseConnection();
        self->WaitForConnectionClose();

        if (self->HasSufficientHeapForCredentialSave()) {
            self->SaveReceivedCredentials(tenantId);
        } else {
            self->LogInsufficientHeapForCredentialSave();
            std::lock_guard<std::mutex> lock(self->mutex_);
            self->status = EnrollmentStatus::Failed_Provision;
        }

        {
            std::lock_guard<std::mutex> lock(self->mutex_);
            self->awaitingMqttClose = false;
        }
        vTaskDelete(nullptr);
    }

    Private Void WaitForConnectionClose() {
        Int waited = 0;
        while (waited < kMqttCloseWaitTimeoutMs) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (mqttConnectionClosed || (!mqttClient && !mqttStarted)) {
                    logger->Info(Tag::Untagged, "MQTT connection close completed");
                    return;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(kMqttCloseWaitStepMs));
            waited += kMqttCloseWaitStepMs;
        }
        logger->Error(Tag::Untagged, "Timed out waiting for MQTT connection close");
    }

    Private Void LogHeapStatus(const StdString& context) const {
        const UInt32 caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
        const size_t freeHeap = esp_get_free_heap_size();
        const size_t minHeap = esp_get_minimum_free_heap_size();
        const size_t largestBlock = heap_caps_get_largest_free_block(caps);

        logger->Info(Tag::Untagged,
                     context + " heap_free=" +
                     StdString(std::to_string(freeHeap).c_str()) +
                     " heap_min=" + StdString(std::to_string(minHeap).c_str()) +
                     " largest_block=" +
                     StdString(std::to_string(largestBlock).c_str()));
    }

    Private size_t EstimateCredentialSaveHeapRequired() const {
        return awsDeviceCertPem.size() + devicePrivateKeyPem.size() +
               thingName.size() + pendingTenantId.size() +
               kCredentialSaveHeapOverheadBytes;
    }

    Private Bool HasSufficientHeapForEnrollment() const {
        const UInt32 caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
        const size_t freeHeap = esp_get_free_heap_size();
        const size_t largestBlock = heap_caps_get_largest_free_block(caps);
        const size_t minContiguousBytes = kEnrollmentHeapRequiredBytes / 2;

        logger->Info(Tag::Untagged,
                     "Enrollment heap check: required=" +
                     StdString(std::to_string(kEnrollmentHeapRequiredBytes).c_str()) +
                     " free=" + StdString(std::to_string(freeHeap).c_str()) +
                     " largest_block=" +
                     StdString(std::to_string(largestBlock).c_str()));

        return freeHeap >= kEnrollmentHeapRequiredBytes &&
               largestBlock >= minContiguousBytes;
    }

    Private Void LogInsufficientHeapForEnrollment() const {
        const UInt32 caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
        const size_t freeHeap = esp_get_free_heap_size();
        const size_t largestBlock = heap_caps_get_largest_free_block(caps);

        logger->Error(Tag::Untagged,
                      "Insufficient heap to start enrollment: required=" +
                      StdString(std::to_string(kEnrollmentHeapRequiredBytes).c_str()) +
                      " free=" + StdString(std::to_string(freeHeap).c_str()) +
                      " largest_block=" +
                      StdString(std::to_string(largestBlock).c_str()));
    }

    Private Bool HasSufficientHeapForCredentialSave() const {
        const size_t requiredBytes = EstimateCredentialSaveHeapRequired();
        const UInt32 caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
        const size_t freeHeap = esp_get_free_heap_size();
        const size_t largestBlock = heap_caps_get_largest_free_block(caps);
        const size_t minContiguousBytes = requiredBytes / 2;

        logger->Info(Tag::Untagged,
                     "Credential save heap check: required=" +
                     StdString(std::to_string(requiredBytes).c_str()) +
                     " free=" + StdString(std::to_string(freeHeap).c_str()) +
                     " largest_block=" +
                     StdString(std::to_string(largestBlock).c_str()));

        return freeHeap >= requiredBytes && largestBlock >= minContiguousBytes;
    }

    Private Void LogInsufficientHeapForCredentialSave() const {
        const size_t requiredBytes = EstimateCredentialSaveHeapRequired();
        const UInt32 caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
        const size_t freeHeap = esp_get_free_heap_size();
        const size_t largestBlock = heap_caps_get_largest_free_block(caps);

        logger->Error(Tag::Untagged,
                      "Insufficient heap to save enrollment credentials: required=" +
                      StdString(std::to_string(requiredBytes).c_str()) +
                      " free=" + StdString(std::to_string(freeHeap).c_str()) +
                      " largest_block=" +
                      StdString(std::to_string(largestBlock).c_str()));
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
    

    Private Void StartMqttClient() {
        if (mqttStarted) return;
        mqttStarted = true;
        enrollmentStarted = false;
        pendingSubscriptions = 0;

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.broker.address.uri = fpProfile.mqttEndpoint.c_str();
        mqtt_cfg.broker.verification.certificate = fpProfile.caCertificatePem.c_str();
        mqtt_cfg.credentials.client_id = deviceService->GetSerialNumber().c_str();
        mqtt_cfg.credentials.authentication.certificate = fpProfile.clientCertificatePem.c_str();
        mqtt_cfg.credentials.authentication.key = fpProfile.clientPrivateKeyPem.c_str();
        mqtt_cfg.buffer.size = 8192;
        mqtt_cfg.buffer.out_size = 8192;
        mqtt_cfg.task.stack_size = 8192;
        mqtt_cfg.network.disable_auto_reconnect = true;

        mqttClient = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_ANY, MqttEventHandler, this);
        esp_mqtt_client_start(mqttClient);

        logger->Info(Tag::Untagged, "MQTT client started for enrollment");
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

        logger->Info(Tag::Untagged, "certificateId: " + StdString(certIdBuf));
        logger->Info(Tag::Untagged, "certificateOwnershipToken: " + ownershipToken);

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

        logger->Info(Tag::Untagged, "========== ENROLLMENT SUCCESS ==========");
        logger->Info(Tag::Untagged, "thingName: " + StdString(thingNameBuf));
        logger->Info(Tag::Untagged, "SerialNumber: " + deviceService->GetSerialNumber());
        logger->Info(Tag::Untagged, "tenantId: " + StdString(tenantIdBuf));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pendingTenantId = StdString(tenantIdBuf);
            thingName = StdString(thingNameBuf);
        }

        // SPIFFS writes and MQTT teardown must not run on mqtt_task (6 KB stack).
        if (xTaskCreate(EnrollmentCompleteTask, "enroll_save", 8192, this, 5, nullptr) != pdPASS) {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            logger->Error(Tag::Untagged, "Failed to schedule credential save task");
        }
    }

    Private Void SaveReceivedCredentials(const StdString& tenantId) {
        logger->Info(Tag::Untagged, "Saving received credentials for tenantId: " + tenantId);

        DeviceIdentityProfileDto identityDto;
        identityDto.clientCertificatePem = Optional<StdString>(awsDeviceCertPem);
        identityDto.clientPrivateKeyPem  = Optional<StdString>(devicePrivateKeyPem);
        identityDto.thingName = Optional<StdString>(thingName);
        identityDto.tenantId = Optional<StdString>(tenantId);

        deviceService->SetDeviceIdentityProfile(identityDto);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Success;
        }

        logger->Info(Tag::Untagged,
                     "Saved device identity profile for serial=" +
                     deviceService->GetSerialNumber());
    }

    Private Static Void MqttEventHandler(Void* handler_args, esp_event_base_t base,
                                         Int32 event_id, Void* event_data) {
        Var self = static_cast<FleetProvisioningService*>(handler_args);
        Var event = static_cast<esp_mqtt_event_handle_t>(event_data);

        switch (event->event_id) {
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
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->status = EnrollmentStatus::Failed_MqttConnect;
                self->logger->Error(Tag::Untagged, "MQTT error during enrollment");
                break;
            }

            case MQTT_EVENT_DISCONNECTED: {
                std::lock_guard<std::mutex> lock(self->mutex_);
                if (self->awaitingMqttClose) {
                    self->mqttConnectionClosed = true;
                    self->logger->Info(Tag::Untagged,
                                       "MQTT disconnected during enrollment teardown");
                } else if (self->status == EnrollmentStatus::InProgress) {
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
