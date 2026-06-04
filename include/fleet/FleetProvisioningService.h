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
#include "esp_heap_caps.h"
#include "sdkconfig.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "StandardDefines.h"
#include "server/IDeviceService.h"
#include "logger/ILogger.h"

// #include "IFleetProvisioningService.h"

/*--@Component--*/
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

        ClearMqttInboundState();
        pendingCreateKeysPayload.clear();
        fpProfile = deviceService->GetFleetProvisioningProfile();
        printf("[Nayan] enroll_topics create_keys_accepted_len=%u provision_accepted_len=%u\n",
               static_cast<unsigned>(fpProfile.createKeysAcceptedTopic.size()),
               static_cast<unsigned>(fpProfile.provisionAcceptedTopic.size()));
        fflush(stdout);
        CloseConnection();
        NayanLogHeap("enroll_start");
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
    static constexpr UInt kEnrollmentMinFreeHeapBytes = 18000;
    static constexpr UInt kEnrollmentMinLargestBlockBytes = 8000;
    static constexpr Int kEnrollmentHeapWaitMs = 8000;
    /** Small MQTT RX buffer; large CreateKeys JSON is reassembled in DispatchMqttInboundMessage. */
    static constexpr Int kEnrollmentMqttBufferSize = 2048;
    static constexpr Int kEnrollmentMqttTaskStackSize = 12288;
    /** Must fit in largest heap block while enrollment MQTT task is still alive. */
    static constexpr Int kEnrollmentSaveTaskStackSize = 8192;
    /** Parse CreateKeys + publish RegisterThing off mqtt_task. */
    static constexpr Int kEnrollmentParseTaskStackSize = 8192;
    static constexpr Size kMaxMqttInboundBytes = 16384;

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
    Private StdString pendingCreateKeysPayload;
    Private StdString mqttInboundReassembly;
    /** Topic from first MQTT DATA fragment (final fragment often has topic_len=0). */
    Private StdString mqttInboundTopic;

    Private Void ClearMqttInboundState() {
        mqttInboundReassembly.clear();
        mqttInboundTopic.clear();
    }

    Private Static Void NayanLogTopicMatch(CChar* label, const StdString& got, const StdString& expect) {
        printf("[Nayan] %s got_len=%u expect_len=%u match=%d\n",
               label,
               static_cast<unsigned>(got.size()),
               static_cast<unsigned>(expect.size()),
               (got == expect) ? 1 : 0);
        fflush(stdout);
    }

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
        printf("[Nayan] enroll_save_task_start\n");
        fflush(stdout);
        self->CloseConnection();
        NayanLogHeap("enroll_after_mqtt_close");
        vTaskDelay(pdMS_TO_TICKS(500));
        NayanLogHeap("enroll_before_spiffs_save");
        self->SaveReceivedCredentials(tenantId);
        vTaskDelete(nullptr);
    }

    Private Static Void CreateKeysProcessTask(Void* arg) {
        Var self = static_cast<FleetProvisioningService*>(arg);
        StdString payload = std::move(self->pendingCreateKeysPayload);
        printf("[Nayan] create_keys_task_start payload_len=%u\n",
               static_cast<unsigned>(payload.size()));
        fflush(stdout);
        NayanLogHeap("create_keys_task_start");
        self->ProcessCreateKeysAccepted(payload.c_str(), static_cast<Int>(payload.size()));
        vTaskDelete(nullptr);
    }

    Private Void ScheduleCreateKeysProcessing(StdString&& payload) {
        printf("[Nayan] create_keys_defer payload_len=%u\n",
               static_cast<unsigned>(payload.size()));
        fflush(stdout);
        pendingCreateKeysPayload = std::move(payload);
        if (xTaskCreate(CreateKeysProcessTask, "enroll_parse", kEnrollmentParseTaskStackSize,
                        this, 8, nullptr) != pdPASS) {
            NayanLogHeap("create_keys_task_fail");
            pendingCreateKeysPayload.clear();
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            logger->Error(Tag::Untagged, "Failed to schedule CreateKeys processing task");
        } else {
            printf("[Nayan] create_keys_task_scheduled ok\n");
            fflush(stdout);
        }
    }

    Private Void DeferCreateKeysProcessing() {
        StdString payloadCopy;
        payloadCopy.swap(mqttInboundReassembly);
        ClearMqttInboundState();
        ScheduleCreateKeysProcessing(std::move(payloadCopy));
    }

    /** After provision accepted: save on enroll_save (closes MQTT off mqtt_task). */
    Private Void ScheduleCredentialSave() {
        ClearMqttInboundState();
        if (xTaskCreate(EnrollmentCompleteTask, "enroll_save", kEnrollmentSaveTaskStackSize,
                        this, 8, nullptr) != pdPASS) {
            NayanLogHeap("enroll_save_task_fail");
            std::lock_guard<std::mutex> lock(mutex_);
            credentialSavePending = false;
            status = EnrollmentStatus::Failed_Provision;
            logger->Error(Tag::Untagged,
                          "Failed to schedule credential save task (need stack="
                          + std::to_string(kEnrollmentSaveTaskStackSize)
                          + " largest_block="
                          + std::to_string(static_cast<ULong>(
                              heap_caps_get_largest_free_block(
                                  MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT))));
        }
    }

    Private bool json_extract_string(const char *json, const char *key, char *out, size_t out_len) {
        char pattern[64];
        snprintf(pattern, sizeof(pattern), "\"%s\"", key);
        const char *keyPos = strstr(json, pattern);
        if (!keyPos) {
            return false;
        }
        const char *colon = strchr(keyPos + strlen(pattern), ':');
        if (!colon) {
            return false;
        }
        const char *start = colon + 1;
        while (*start == ' ' || *start == '\t') {
            ++start;
        }
        if (*start != '"') {
            return false;
        }
        ++start;
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

    Private Static Void NayanLogHeap(CChar* where) {
        const UInt32 caps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
        printf("[Nayan] %s heap_free=%lu heap_min=%lu largest_block=%lu\n",
            where,
            (unsigned long)esp_get_free_heap_size(),
            (unsigned long)esp_get_minimum_free_heap_size(),
            (unsigned long)heap_caps_get_largest_free_block(caps));
        fflush(stdout);
    }

    Private Void WaitForEnrollmentHeap() {
        NayanLogHeap("heap_wait_start");
        Const Int stepMs = 200;
        Int waited = 0;
        while (waited < kEnrollmentHeapWaitMs) {
            multi_heap_info_t info{};
            heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            if (info.total_free_bytes >= kEnrollmentMinFreeHeapBytes
                && info.largest_free_block >= kEnrollmentMinLargestBlockBytes) {
                NayanLogHeap("heap_wait_ok");
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(stepMs));
            waited += stepMs;
        }
        NayanLogHeap("heap_wait_timeout");
        logger->Warning(Tag::Untagged,
                        "Enrollment heap still low before TLS: free="
                        + std::to_string(static_cast<ULong>(esp_get_free_heap_size()))
                        + " largest_block="
                        + std::to_string(static_cast<ULong>(
                            heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT))));
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
        mqtt_cfg.buffer.size = kEnrollmentMqttBufferSize;
        mqtt_cfg.buffer.out_size = kEnrollmentMqttBufferSize;
        mqtt_cfg.task.stack_size = kEnrollmentMqttTaskStackSize;

        // Allow DNS/route to AWS IoT to settle (internet check does not validate *.amazonaws.com).
        vTaskDelay(pdMS_TO_TICKS(2000));
        WaitForEnrollmentHeap();
        NayanLogHeap("before_mqtt_init");

        mqttClient = esp_mqtt_client_init(&mqtt_cfg);
        if (!mqttClient) {
            logger->Error(Tag::Untagged,
                "Enrollment MQTT init failed host=" + enrollmentBrokerHost);
            mqttStarted = false;
            return;
        }
        NayanLogHeap("after_mqtt_init");
        WaitForEnrollmentHeap();

        esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_ANY, MqttEventHandler, this);
        const esp_err_t startErr = esp_mqtt_client_start(mqttClient);
        NayanLogHeap("after_mqtt_start");
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

    Private Void ProcessCreateKeysAccepted(CChar* payload, Int len) {
        logger->Info(Tag::Untagged, "CreateKeysAndCertificate ACCEPTED");
        printf("[Nayan] create_keys_accepted payload_len=%d\n", len);
        fflush(stdout);

        if (!json_extract_string(payload, "certificateOwnershipToken", ownershipBuf, sizeof(ownershipBuf))) {
            printf("[Nayan] create_keys_parse_fail token_field=%s\n",
                strstr(payload, "certificateOwnershipToken") ? "present" : "absent");
            fflush(stdout);
            logger->Error(Tag::Untagged,
                          "Missing certificateOwnershipToken (payload_len=" + std::to_string(len) + ")");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            return;
        }
        if (!json_extract_string(payload, "certificateId", certIdBuf, sizeof(certIdBuf))) {
            logger->Error(Tag::Untagged, "Missing certificateId");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            return;
        }
        if (!json_extract_string(payload, "privateKey", keyEscBuf, sizeof(keyEscBuf))) {
            logger->Error(Tag::Untagged, "Missing privateKey");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            return;
        }
        if (!json_extract_string(payload, "certificatePem", certEscBuf, sizeof(certEscBuf))) {
            logger->Error(Tag::Untagged, "Missing certificatePem");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            return;
        }

        unescape_json_to_pem(keyEscBuf, keyPemBuf, sizeof(keyPemBuf));
        unescape_json_to_pem(certEscBuf, certPemBuf, sizeof(certPemBuf));

        ownershipToken      = ownershipBuf;
        devicePrivateKeyPem = keyPemBuf;
        awsDeviceCertPem    = certPemBuf;

        NayanLogHeap("create_keys_parsed");
        PublishProvisionRequest();
    }

    Private Void PublishProvisionRequest() {
        if (!mqttClient) {
            printf("[Nayan] publish_register_thing skip mqtt_client=null\n");
            fflush(stdout);
            logger->Error(Tag::Untagged, "Cannot publish RegisterThing: MQTT client not running");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            return;
        }

        StdString json = "{\"certificateOwnershipToken\":\"" + ownershipToken +
                         "\",\"parameters\":{\"SerialNumber\":\"" +
                         deviceService->GetSerialNumber() + "\"}}";
        logger->Info(Tag::Untagged, "Publishing RegisterThing for serial=" +
                                    deviceService->GetSerialNumber());
        NayanLogHeap("before_publish_register");
        const Int msgId = esp_mqtt_client_publish(
            mqttClient, fpProfile.provisionRequestTopic.c_str(),
            json.c_str(), static_cast<Int>(json.size()), 1, 0);
        printf("[Nayan] publish_register_thing msg_id=%d json_len=%u\n",
               msgId, static_cast<unsigned>(json.size()));
        fflush(stdout);
        if (msgId < 0) {
            logger->Error(Tag::Untagged, "RegisterThing publish failed");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
        }
    }

    Private Void DispatchMqttInboundMessage(esp_mqtt_event_handle_t event) {
        if (!event || !event->data || event->data_len <= 0) {
            printf("[Nayan] mqtt_data_skip null_or_empty\n");
            fflush(stdout);
            return;
        }

        const Int totalLen = event->total_data_len > 0 ? event->total_data_len : event->data_len;
        if (totalLen <= 0 || static_cast<Size>(totalLen) > kMaxMqttInboundBytes) {
            logger->Error(Tag::Untagged,
                          "Enrollment MQTT payload too large: " + std::to_string(totalLen));
            ClearMqttInboundState();
            return;
        }

        if (event->current_data_offset == 0) {
            ClearMqttInboundState();
            mqttInboundReassembly.reserve(static_cast<Size>(totalLen));
            if (event->topic_len > 0 && event->topic) {
                mqttInboundTopic.assign(event->topic, static_cast<Size>(event->topic_len));
                printf("[Nayan] mqtt_fragment_first offset=0 chunk=%d total=%d saved_topic_len=%u\n",
                       event->data_len, totalLen,
                       static_cast<unsigned>(mqttInboundTopic.size()));
                fflush(stdout);
            } else {
                printf("[Nayan] mqtt_fragment_first offset=0 chunk=%d total=%d saved_topic_len=0\n",
                       event->data_len, totalLen);
                fflush(stdout);
            }
        } else {
            printf("[Nayan] mqtt_fragment_cont offset=%d chunk=%d total=%d event_topic_len=%d\n",
                   event->current_data_offset, event->data_len, totalLen, event->topic_len);
            fflush(stdout);
        }

        if (event->current_data_offset != static_cast<Int>(mqttInboundReassembly.size())) {
            ClearMqttInboundState();
            printf("[Nayan] mqtt_fragment_out_of_order offset=%d have=%u\n",
                   event->current_data_offset,
                   static_cast<unsigned>(mqttInboundReassembly.size()));
            fflush(stdout);
            logger->Error(Tag::Untagged, "Enrollment MQTT payload fragment out of order");
            return;
        }

        mqttInboundReassembly.append(event->data, static_cast<Size>(event->data_len));

        if (static_cast<Int>(mqttInboundReassembly.size()) < totalLen) {
            printf("[Nayan] mqtt_reassembly_progress have=%u need=%d\n",
                   static_cast<unsigned>(mqttInboundReassembly.size()), totalLen);
            fflush(stdout);
            return;
        }

        StdString topic = mqttInboundTopic;
        if (topic.empty() && event->topic_len > 0 && event->topic) {
            topic.assign(event->topic, static_cast<Size>(event->topic_len));
        }

        printf("[Nayan] mqtt_payload_complete event_topic_len=%d saved_topic_len=%u payload_len=%d\n",
               event->topic_len, static_cast<unsigned>(topic.size()), totalLen);
        fflush(stdout);

        const Bool hasOwnershipToken =
            strstr(mqttInboundReassembly.c_str(), "certificateOwnershipToken") != nullptr;
        const Bool hasThingName = strstr(mqttInboundReassembly.c_str(), "thingName") != nullptr;

        if (topic == fpProfile.createKeysAcceptedTopic) {
            NayanLogTopicMatch("route_create_keys_topic", topic, fpProfile.createKeysAcceptedTopic);
            DeferCreateKeysProcessing();
            return;
        }
        if (topic.empty() && hasOwnershipToken) {
            printf("[Nayan] route_create_keys_payload_fallback\n");
            fflush(stdout);
            DeferCreateKeysProcessing();
            return;
        }

        const char* payload = mqttInboundReassembly.c_str();
        const Int payloadLen = static_cast<Int>(mqttInboundReassembly.size());

        if (topic == fpProfile.provisionAcceptedTopic) {
            NayanLogTopicMatch("route_provision_accepted", topic, fpProfile.provisionAcceptedTopic);
            HandleProvisionAccepted(payload, payloadLen);
        } else if (topic == fpProfile.provisionRejectedTopic) {
            printf("[Nayan] route_provision_rejected\n");
            fflush(stdout);
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            logger->Error(Tag::Untagged, "Provisioning rejected");
        } else if (topic.empty() && hasThingName) {
            printf("[Nayan] route_provision_accepted_payload_fallback\n");
            fflush(stdout);
            HandleProvisionAccepted(payload, payloadLen);
        } else {
            printf("[Nayan] route_unknown topic_len=%u has_token=%d has_thing=%d\n",
                   static_cast<unsigned>(topic.size()), hasOwnershipToken ? 1 : 0,
                   hasThingName ? 1 : 0);
            fflush(stdout);
            logger->Error(Tag::Untagged,
                          "Enrollment MQTT message on unknown topic (len="
                          + std::to_string(static_cast<Int>(topic.size())) + ")");
        }

        ClearMqttInboundState();
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

        NayanLogHeap("provision_accepted");
        printf("[Nayan] provision_accepted_defer_save thing=%s\n", thingNameBuf);
        fflush(stdout);
        ScheduleCredentialSave();
    }

    Private Void SaveReceivedCredentials(const StdString& tenantId) {
        logger->Info(Tag::Untagged, "Saving received credentials for tenantId: " + tenantId);

        const StdString savedThingName = thingName;
        DeviceIdentityProfileDto identityDto;
        identityDto.mqttEndpoint = Optional<StdString>(fpProfile.mqttEndpoint);
        identityDto.caCertificatePem = Optional<StdString>(fpProfile.caCertificatePem);
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

        const auto profile = deviceService->GetDeviceIdentityProfile();
        printf("[Nayan] post_enroll_save done thing=%s tenant=%s profile_present=%d "
               "endpoint_len=%u cert_len=%u key_len=%u cmd_topic_len=%u\n",
               savedThingName.c_str(),
               tenantId.c_str(),
               profile.has_value() ? 1 : 0,
               profile.has_value() ? static_cast<unsigned>(profile->mqttEndpoint.size()) : 0U,
               profile.has_value() ? static_cast<unsigned>(profile->clientCertificatePem.size()) : 0U,
               profile.has_value() ? static_cast<unsigned>(profile->clientPrivateKeyPem.size()) : 0U,
               profile.has_value()
                   ? static_cast<unsigned>(profile->subscribeTopics.commandTopic.size())
                   : 0U);
        fflush(stdout);

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

            case MQTT_EVENT_DATA:
                self->DispatchMqttInboundMessage(event);
                break;

            case MQTT_EVENT_ERROR: {
                Bool ignoreTeardownError = false;
                {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    ignoreTeardownError = self->credentialSavePending;
                }
                if (ignoreTeardownError) {
                    break;
                }
                printf("[Nayan] mqtt_error during_enrollment\n");
                fflush(stdout);
                NayanLogHeap("mqtt_error");
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
                    printf("[Nayan] mqtt_disconnected during_enrollment\n");
                    fflush(stdout);
                    NayanLogHeap("mqtt_disconnected");
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
