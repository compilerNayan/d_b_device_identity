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
#include "communication/IServerProvider.h"

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

    /* @Autowired */
    Private IServerProviderPtr serverProvider;

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
    static constexpr size_t kOwnershipBufBytes = 768;
    static constexpr size_t kCertIdBufBytes = 128;
    static constexpr size_t kPemEscBufBytes = 4096;
    static constexpr size_t kPemBufBytes = 4096;
    static constexpr size_t kProvisionFieldBufBytes = 128;
    static constexpr UInt32 kParseHeapCaps = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;

    // Credential strings held between CreateKeys/RegisterThing and SPIFFS save.
    Private StdString devicePrivateKeyPem;
    Private StdString awsDeviceCertPem;
    Private StdString awsCaCertificatePem;
    Private StdString ownershipToken;
    Private StdString thingName;
    Private StdString pendingTenantId;

    /** Scoped heap parse buffers; avoids ~17 KB permanent member footprint and mqtt_task stack use. */
    struct CreateKeysParseBuffers {
        Char* ownershipBuf = nullptr;
        Char* certIdBuf = nullptr;
        Char* keyEscBuf = nullptr;
        Char* certEscBuf = nullptr;
        Char* keyPemBuf = nullptr;
        Char* certPemBuf = nullptr;
        const Bool ok;

        CreateKeysParseBuffers()
            : ok(AllocateAll()) {}

        ~CreateKeysParseBuffers() {
            FreeAll();
        }

        CreateKeysParseBuffers(const CreateKeysParseBuffers&) = delete;
        CreateKeysParseBuffers& operator=(const CreateKeysParseBuffers&) = delete;

    Private
        static Char* Alloc(size_t bytes) {
            return static_cast<Char*>(heap_caps_malloc(bytes, kParseHeapCaps));
        }

        static Void Free(Char* buf) {
            if (buf) {
                heap_caps_free(buf);
            }
        }

        Bool AllocateAll() {
            ownershipBuf = Alloc(kOwnershipBufBytes);
            certIdBuf = Alloc(kCertIdBufBytes);
            keyEscBuf = Alloc(kPemEscBufBytes);
            certEscBuf = Alloc(kPemEscBufBytes);
            keyPemBuf = Alloc(kPemBufBytes);
            certPemBuf = Alloc(kPemBufBytes);
            if (ownershipBuf && certIdBuf && keyEscBuf && certEscBuf && keyPemBuf && certPemBuf) {
                return true;
            }
            FreeAll();
            return false;
        }

        Void FreeAll() {
            Free(ownershipBuf);
            ownershipBuf = nullptr;
            Free(certIdBuf);
            certIdBuf = nullptr;
            Free(keyEscBuf);
            keyEscBuf = nullptr;
            Free(certEscBuf);
            certEscBuf = nullptr;
            Free(keyPemBuf);
            keyPemBuf = nullptr;
            Free(certPemBuf);
            certPemBuf = nullptr;
        }
    };

    struct ProvisionParseBuffers {
        Char* thingNameBuf = nullptr;
        Char* tenantIdBuf = nullptr;
        const Bool ok;

        ProvisionParseBuffers()
            : ok(AllocateAll()) {}

        ~ProvisionParseBuffers() {
            FreeAll();
        }

        ProvisionParseBuffers(const ProvisionParseBuffers&) = delete;
        ProvisionParseBuffers& operator=(const ProvisionParseBuffers&) = delete;

    Private
        static Char* Alloc(size_t bytes) {
            return static_cast<Char*>(heap_caps_malloc(bytes, kParseHeapCaps));
        }

        static Void Free(Char* buf) {
            if (buf) {
                heap_caps_free(buf);
            }
        }

        Bool AllocateAll() {
            thingNameBuf = Alloc(kProvisionFieldBufBytes);
            tenantIdBuf = Alloc(kProvisionFieldBufBytes);
            if (thingNameBuf && tenantIdBuf) {
                return true;
            }
            FreeAll();
            return false;
        }

        Void FreeAll() {
            Free(thingNameBuf);
            thingNameBuf = nullptr;
            Free(tenantIdBuf);
            tenantIdBuf = nullptr;
        }
    };

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
        CreateKeysParseBuffers parseBufs;
        if (!parseBufs.ok) {
            logger->Error(Tag::Untagged,
                          "Insufficient heap for CreateKeys JSON parse buffers");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_InsufficientHeap;
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        logger->Info(Tag::Untagged, "CreateKeysAndCertificate ACCEPTED");

        StdString payloadStr(payload, len);

        if (!json_extract_string(payloadStr.c_str(), "certificateOwnershipToken",
                                 parseBufs.ownershipBuf, kOwnershipBufBytes)) {
            logger->Error(Tag::Untagged, "Missing certificateOwnershipToken");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificateId",
                                 parseBufs.certIdBuf, kCertIdBufBytes)) {
            logger->Error(Tag::Untagged, "Missing certificateId");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "privateKey",
                                 parseBufs.keyEscBuf, kPemEscBufBytes)) {
            logger->Error(Tag::Untagged, "Missing privateKey");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificatePem",
                                 parseBufs.certEscBuf, kPemEscBufBytes)) {
            logger->Error(Tag::Untagged, "Missing certificatePem");
            return;
        }

        unescape_json_to_pem(parseBufs.keyEscBuf, parseBufs.keyPemBuf, kPemBufBytes);
        unescape_json_to_pem(parseBufs.certEscBuf, parseBufs.certPemBuf, kPemBufBytes);

        ownershipToken      = parseBufs.ownershipBuf;
        devicePrivateKeyPem = parseBufs.keyPemBuf;
        awsDeviceCertPem    = parseBufs.certPemBuf;

        logger->Info(Tag::Untagged, "certificateId: " + StdString(parseBufs.certIdBuf));
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
        ProvisionParseBuffers parseBufs;
        if (!parseBufs.ok) {
            logger->Error(Tag::Untagged,
                          "Insufficient heap for provision JSON parse buffers");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_InsufficientHeap;
            return;
        }

        StdString payloadStr(payload, len);

        if (!json_extract_string(payloadStr.c_str(), "thingName",
                                 parseBufs.thingNameBuf, kProvisionFieldBufBytes)) {
            logger->Error(Tag::Untagged, "Missing thingName in provision response");
            return;
        }

        if (!json_extract_string(payloadStr.c_str(), "tenantId",
                                 parseBufs.tenantIdBuf, kProvisionFieldBufBytes)) {
            logger->Error(Tag::Untagged, "Missing tenantId in provision response");
            return;
        }

        logger->Info(Tag::Untagged, "========== ENROLLMENT SUCCESS ==========");
        logger->Info(Tag::Untagged, "thingName: " + StdString(parseBufs.thingNameBuf));
        logger->Info(Tag::Untagged, "SerialNumber: " + deviceService->GetSerialNumber());
        logger->Info(Tag::Untagged, "tenantId: " + StdString(parseBufs.tenantIdBuf));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pendingTenantId = StdString(parseBufs.tenantIdBuf);
            thingName = StdString(parseBufs.thingNameBuf);
        }

        // SPIFFS writes and MQTT teardown must not run on mqtt_task (6 KB stack).
        if (xTaskCreate(EnrollmentCompleteTask, "enroll_save", 8192, this, 5, nullptr) != pdPASS) {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            logger->Error(Tag::Untagged, "Failed to schedule credential save task");
        }
    }

    Private Void ClearEnrollmentCredentialStrings() {
        awsDeviceCertPem = "";
        devicePrivateKeyPem = "";
        thingName = "";
        pendingTenantId = "";
        ownershipToken = "";
    }

    Private Void SaveReceivedCredentials(const StdString& tenantId) {
        logger->Info(Tag::Untagged, "Saving received credentials for tenantId: " + tenantId);

        const StdString savedThingName = thingName;
        const StdString serialNumber = deviceService->GetSerialNumber();

        DeviceIdentityProfileDto identityDto;
        identityDto.clientCertificatePem = Optional<StdString>(awsDeviceCertPem);
        identityDto.clientPrivateKeyPem  = Optional<StdString>(devicePrivateKeyPem);
        identityDto.thingName = Optional<StdString>(thingName);
        identityDto.tenantId = Optional<StdString>(tenantId);

        deviceService->SetDeviceIdentityProfile(identityDto);
        PublishEnrollmentCompleteMessage(tenantId, savedThingName, serialNumber);
        ClearEnrollmentCredentialStrings();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Success;
        }

        logger->Info(Tag::Untagged,
                     "Saved device identity profile for serial=" +
                     deviceService->GetSerialNumber());
    }

    Private Void PublishEnrollmentCompleteMessage(
            const StdString& tenantId,
            const StdString& deviceId,
            const StdString& serialNumber) {

        ICloudServerPtr cloudServer = serverProvider->GetCloudServerPtr();

        if (cloudServer == nullptr) {
            logger->Warning(Tag::Untagged, "Cloud server unavailable; skipping lifecycle/enrolled publish");
            return;
        }

        const StdString enrolledAt = FormatUtcTimestamp();
        const StdString payload =
            "{\"tenantId\":\"" + tenantId +
            "\",\"deviceId\":\"" + deviceId +
            "\",\"serialNumber\":\"" + serialNumber +
            "\",\"enrolledAt\":\"" + enrolledAt + "\"}";

        if (cloudServer->PublishEnrollmentComplete(payload)) {
            logger->Info(Tag::Untagged, "Queued lifecycle/enrolled publish");
        } else {
            logger->Warning(Tag::Untagged, "Failed to queue lifecycle/enrolled publish");
        }
    }

    Private Static StdString FormatUtcTimestamp() {
        time_t now = time(nullptr);
        struct tm utc;
        gmtime_r(&now, &utc);
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
        return StdString(buffer);
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
