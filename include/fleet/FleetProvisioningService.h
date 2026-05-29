#ifndef FLEETPROVISIONINGSERVICE_H
#define FLEETPROVISIONINGSERVICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <sys/time.h>
#include <mutex>

#include "StandardDefines.h"
#include "../service/IDeviceService.h"
#include "logger/ILogger.h"

#include "IFleetProvisioningService.h"

/* @Component */
class FleetProvisioningService : public IFleetProvisioningService {
    
    Public FleetProvisioningService()
        : mqttClient(nullptr),
          mqttStarted(false),
          enrollmentStarted(false),
          pendingSubscriptions(0),
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
    Private std::mutex mutex_;
    Private EnrollmentStatus status;

    // Buffers for received credentials
    Private StdString devicePrivateKeyPem;
    Private StdString awsDeviceCertPem;
    Private StdString awsCaCertificatePem;
    Private StdString ownershipToken;
    Private StdString thingName;

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
    
        // Copy payload into a buffer
        StdString payloadStr(payload, len);
    
        // Extract fields (using same helpers as in your C code)
        Char ownershipBuf[768];
        Char certIdBuf[128];
        Char keyEsc[4096];
        Char certEsc[4096];
    
        if (!json_extract_string(payloadStr.c_str(), "certificateOwnershipToken", ownershipBuf, sizeof(ownershipBuf))) {
            logger->Error(Tag::Untagged, "Missing certificateOwnershipToken");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificateId", certIdBuf, sizeof(certIdBuf))) {
            logger->Error(Tag::Untagged, "Missing certificateId");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "privateKey", keyEsc, sizeof(keyEsc))) {
            logger->Error(Tag::Untagged, "Missing privateKey");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificatePem", certEsc, sizeof(certEsc))) {
            logger->Error(Tag::Untagged, "Missing certificatePem");
            return;
        }
    
        // Unescape JSON strings into PEM format
        Char keyPem[4096];
        Char certPem[4096];
        unescape_json_to_pem(keyEsc, keyPem, sizeof(keyPem));
        unescape_json_to_pem(certEsc, certPem, sizeof(certPem));
    
        // Save into class members
        ownershipToken     = ownershipBuf;
        devicePrivateKeyPem = keyPem;
        awsDeviceCertPem    = certPem;
    
        logger->Info(Tag::Untagged, "certificateId: " + StdString(certIdBuf));
        logger->Info(Tag::Untagged, "certificateOwnershipToken: " + ownershipToken);
    
        // Continue with RegisterThing
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
        std::lock_guard<std::mutex> lock(mutex_);
    
        StdString payloadStr(payload, len);
    
        Char thingNameBuf[128];
        if (!json_extract_string(payloadStr.c_str(), "thingName", thingNameBuf, sizeof(thingNameBuf))) {
            logger->Error(Tag::Untagged, "Missing thingName in provision response");
            return;
        }
    
        logger->Info(Tag::Untagged, "========== ENROLLMENT SUCCESS ==========");
        logger->Info(Tag::Untagged, "thingName: " + StdString(thingNameBuf));
        logger->Info(Tag::Untagged, "SerialNumber: " + deviceService->GetSerialNumber());
    
        // At this point, devicePrivateKeyPem and awsDeviceCertPem are already populated
        SaveReceivedCredentials();
    }
    
    Private Void SaveReceivedCredentials() {
        std::lock_guard<std::mutex> lock(mutex_);

        DeviceIdentityProfileDto identityDto;
        identityDto.clientCertificatePem = Optional<StdString>(awsDeviceCertPem);
        identityDto.clientPrivateKeyPem  = Optional<StdString>(devicePrivateKeyPem);

        deviceService->SetDeviceIdentityProfile(identityDto);

        status = EnrollmentStatus::Success;

        logger->Info(Tag::Untagged,
                     "Saved device identity profile for serial=" +
                     deviceService->GetSerialNumber());

        CloseConnection();
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
