#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "IDeviceService.h"
#include "../repository/FleetProvisioningProfileRepository.h"
#include "../repository/DeviceIdentityProfileRepository.h"
#include "ConnectionConstants.h"

/* @Service */
class DeviceService : public IDeviceService {
    Public DeviceService() {
        Refresh();
    }
    Public Virtual ~DeviceService() = default;

    /* @Autowired */
    Private FleetProvisioningProfileRepositoryPtr fleetProvisioningProfileRepository;

    /* @Autowired */
    Private DeviceIdentityProfileRepositoryPtr deviceIdentityProfileRepository;

    Private StdString serialNumber;
    Private StdString deviceSecret;
    Private StdString firmwareVersion;

    Private FleetProvisioningProfileData fleetProvisioningProfile;
    Private Optional<DeviceIdentityProfileData> deviceIdentityProfile;

    Private mutable std::mutex mutex_;

    Public Void Refresh() override {
        Val fleetProvisioningProfileOpt = fleetProvisioningProfileRepository->FindFirst();
        Val deviceIdentityProfileOpt = deviceIdentityProfileRepository->FindFirst();

        std::lock_guard<std::mutex> lock(mutex_);
        serialNumber = "1234";
        deviceSecret = "1234";
        firmwareVersion = "1.0.0";

        this->fleetProvisioningProfile.mqttEndpoint = FLEET_PROVISIONING_MQTT_ENDPOINT;
        this->fleetProvisioningProfile.caCertificatePem = FLEET_PROVISIONING_CA_CERTIFICATE_PEM;
        this->fleetProvisioningProfile.clientCertificatePem = FLEET_PROVISIONING_CLIENT_CERTIFICATE_PEM;
        this->fleetProvisioningProfile.clientPrivateKeyPem = FLEET_PROVISIONING_CLIENT_PRIVATE_KEY_PEM;
        this->fleetProvisioningProfile.createKeysRequestTopic = FLEET_PROVISIONING_CREATE_KEYS_REQUEST_TOPIC;
        this->fleetProvisioningProfile.createKeysAcceptedTopic = FLEET_PROVISIONING_CREATE_KEYS_ACCEPTED_TOPIC;
        this->fleetProvisioningProfile.createKeysRejectedTopic = FLEET_PROVISIONING_CREATE_KEYS_REJECTED_TOPIC;
        this->fleetProvisioningProfile.provisionRequestTopic = FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC;
        this->fleetProvisioningProfile.provisionAcceptedTopic = FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC;

        if (fleetProvisioningProfileOpt.has_value()) {
            Val fleetProvisioningProfileEntity = fleetProvisioningProfileOpt.value();
            if(fleetProvisioningProfileEntity.mqttEndpoint.has_value()) this->fleetProvisioningProfile.mqttEndpoint = fleetProvisioningProfileEntity.mqttEndpoint.value();
            if(fleetProvisioningProfileEntity.caCertificatePem.has_value()) this->fleetProvisioningProfile.caCertificatePem = fleetProvisioningProfileEntity.caCertificatePem.value();
            if(fleetProvisioningProfileEntity.clientCertificatePem.has_value()) this->fleetProvisioningProfile.clientCertificatePem = fleetProvisioningProfileEntity.clientCertificatePem.value();
            if(fleetProvisioningProfileEntity.clientPrivateKeyPem.has_value()) this->fleetProvisioningProfile.clientPrivateKeyPem = fleetProvisioningProfileEntity.clientPrivateKeyPem.value();
            if(fleetProvisioningProfileEntity.createKeysRequestTopic.has_value()) this->fleetProvisioningProfile.createKeysRequestTopic = fleetProvisioningProfileEntity.createKeysRequestTopic.value();
            if(fleetProvisioningProfileEntity.createKeysAcceptedTopic.has_value()) this->fleetProvisioningProfile.createKeysAcceptedTopic = fleetProvisioningProfileEntity.createKeysAcceptedTopic.value();
            if(fleetProvisioningProfileEntity.createKeysRejectedTopic.has_value()) this->fleetProvisioningProfile.createKeysRejectedTopic = fleetProvisioningProfileEntity.createKeysRejectedTopic.value();
            if(fleetProvisioningProfileEntity.provisionRequestTopic.has_value()) this->fleetProvisioningProfile.provisionRequestTopic = fleetProvisioningProfileEntity.provisionRequestTopic.value();
            if(fleetProvisioningProfileEntity.provisionAcceptedTopic.has_value()) this->fleetProvisioningProfile.provisionAcceptedTopic = fleetProvisioningProfileEntity.provisionAcceptedTopic.value();
            if(fleetProvisioningProfileEntity.provisionRejectedTopic.has_value()) this->fleetProvisioningProfile.provisionRejectedTopic = fleetProvisioningProfileEntity.provisionRejectedTopic.value();
        }

        this->deviceIdentityProfile->mqttEndpoint = DEVICE_IDENTITY_MQTT_ENDPOINT;
        this->deviceIdentityProfile->caCertificatePem = DEVICE_IDENTITY_CA_CERTIFICATE_PEM;
        
        this->deviceIdentityProfile->publishTopics.statusTopic = DEVICE_IDENTITY_PUBLISH_TOPICS_STATUS_TOPIC;
        this->deviceIdentityProfile->publishTopics.telemetryTopic = DEVICE_IDENTITY_PUBLISH_TOPICS_TELEMETRY_TOPIC;
        this->deviceIdentityProfile->publishTopics.logsTopic = DEVICE_IDENTITY_PUBLISH_TOPICS_LOGS_TOPIC;
        this->deviceIdentityProfile->publishTopics.eventsTopic = DEVICE_IDENTITY_PUBLISH_TOPICS_EVENTS_TOPIC;

        this->deviceIdentityProfile->subscribeTopics.commandTopic = DEVICE_IDENTITY_SUBSCRIBE_TOPICS_COMMAND_TOPIC;
        this->deviceIdentityProfile->subscribeTopics.otaUpdateTopic = DEVICE_IDENTITY_SUBSCRIBE_TOPICS_OTA_UPDATE_TOPIC;
        this->deviceIdentityProfile->subscribeTopics.featureFlagTopic = DEVICE_IDENTITY_SUBSCRIBE_TOPICS_FEATURE_FLAG_TOPIC;

        if(deviceIdentityProfileOpt.has_value()) {
            if(deviceIdentityProfileOpt->clientCertificatePem.has_value() &&
                deviceIdentityProfileOpt->clientPrivateKeyPem.has_value()) {
                Val deviceIdentityProfileEntity = deviceIdentityProfileOpt.value();
                if(deviceIdentityProfileEntity.mqttEndpoint.has_value()) this->deviceIdentityProfile->mqttEndpoint = deviceIdentityProfileEntity.mqttEndpoint.value();
                if(deviceIdentityProfileEntity.caCertificatePem.has_value()) this->deviceIdentityProfile->caCertificatePem = deviceIdentityProfileEntity.caCertificatePem.value();
                if(deviceIdentityProfileEntity.clientCertificatePem.has_value()) this->deviceIdentityProfile->clientCertificatePem = deviceIdentityProfileEntity.clientCertificatePem.value();
                if(deviceIdentityProfileEntity.clientPrivateKeyPem.has_value()) this->deviceIdentityProfile->clientPrivateKeyPem = deviceIdentityProfileEntity.clientPrivateKeyPem.value();
                if(deviceIdentityProfileEntity.publishTopics.has_value()) {
                    Val publishTopicsEntity = deviceIdentityProfileEntity.publishTopics.value();
                    if(publishTopicsEntity.statusTopic.has_value()) this->deviceIdentityProfile->publishTopics.statusTopic = publishTopicsEntity.statusTopic.value();
                    if(publishTopicsEntity.telemetryTopic.has_value()) this->deviceIdentityProfile->publishTopics.telemetryTopic = publishTopicsEntity.telemetryTopic.value();
                    if(publishTopicsEntity.logsTopic.has_value()) this->deviceIdentityProfile->publishTopics.logsTopic = publishTopicsEntity.logsTopic.value();
                    if(publishTopicsEntity.eventsTopic.has_value()) this->deviceIdentityProfile->publishTopics.eventsTopic = publishTopicsEntity.eventsTopic.value();
                }
                if(deviceIdentityProfileEntity.subscribeTopics.has_value()) {
                    Val subscribeTopicsEntity = deviceIdentityProfileEntity.subscribeTopics.value();
                    if(subscribeTopicsEntity.commandTopic.has_value()) this->deviceIdentityProfile->subscribeTopics.commandTopic = subscribeTopicsEntity.commandTopic.value();
                    if(subscribeTopicsEntity.otaUpdateTopic.has_value()) this->deviceIdentityProfile->subscribeTopics.otaUpdateTopic = subscribeTopicsEntity.otaUpdateTopic.value();
                    if(subscribeTopicsEntity.featureFlagTopic.has_value()) this->deviceIdentityProfile->subscribeTopics.featureFlagTopic = subscribeTopicsEntity.featureFlagTopic.value();
                }

            } else {
                this->deviceIdentityProfile = std::nullopt;
            }
        } else {
            this->deviceIdentityProfile = std::nullopt;
        }
    }

    // Implemented interface methods with locking
    Public StdString GetSerialNumber() const override { std::lock_guard<std::mutex> lock(mutex_); return serialNumber; }
    Public StdString GetDeviceSecret() const override { std::lock_guard<std::mutex> lock(mutex_); return deviceSecret; }
    Public StdString GetFirmwareVersion() const override { std::lock_guard<std::mutex> lock(mutex_); return firmwareVersion; }

    Public FleetProvisioningProfileData GetFleetProvisioningProfile() const override { std::lock_guard<std::mutex> lock(mutex_); return fleetProvisioningProfile; }
    Public Optional<DeviceIdentityProfileData> GetDeviceIdentityProfile() const override { std::lock_guard<std::mutex> lock(mutex_); return deviceIdentityProfile; }

    Public Void SetFleetProvisioningProfile(const FleetProvisioningProfileDto& fleetProvisioningProfileDto) override { 
        Var fleetProvisioningProfileEntity = GetFleetProvisioningProfileEntity(fleetProvisioningProfileDto);
        fleetProvisioningProfileRepository->UpdateAvailableFields(fleetProvisioningProfileEntity);
        Refresh();
    }
    
    Public Void SetDeviceIdentityProfile(const DeviceIdentityProfileDto& deviceIdentityProfileDto) override { 
        Var deviceIdentityProfileEntity = GetDeviceIdentityProfileEntity(deviceIdentityProfileDto);
        deviceIdentityProfileRepository->UpdateAvailableFields(deviceIdentityProfileEntity);
        Refresh();
    }

    Private FleetProvisioningProfile GetFleetProvisioningProfileEntity(const FleetProvisioningProfileDto& fleetProvisioningProfileDto) {
        FleetProvisioningProfile fleetProvisioningProfileEntity;
        fleetProvisioningProfileEntity.id = 1;
        fleetProvisioningProfileEntity.mqttEndpoint = fleetProvisioningProfileDto.mqttEndpoint.value();
        fleetProvisioningProfileEntity.caCertificatePem = fleetProvisioningProfileDto.caCertificatePem.value();
        fleetProvisioningProfileEntity.clientCertificatePem = fleetProvisioningProfileDto.clientCertificatePem.value();
        fleetProvisioningProfileEntity.clientPrivateKeyPem = fleetProvisioningProfileDto.clientPrivateKeyPem.value();
        fleetProvisioningProfileEntity.createKeysRequestTopic = fleetProvisioningProfileDto.createKeysRequestTopic.value();
        fleetProvisioningProfileEntity.createKeysAcceptedTopic = fleetProvisioningProfileDto.createKeysAcceptedTopic.value();
        fleetProvisioningProfileEntity.createKeysRejectedTopic = fleetProvisioningProfileDto.createKeysRejectedTopic.value();
        fleetProvisioningProfileEntity.provisionRequestTopic = fleetProvisioningProfileDto.provisionRequestTopic.value();
        fleetProvisioningProfileEntity.provisionAcceptedTopic = fleetProvisioningProfileDto.provisionAcceptedTopic.value();
        //fleetProvisioningProfileEntity.provisionRejectedTopic = fleetProvisioningProfileDto.provisionRejectedTopic.value();
        return fleetProvisioningProfileEntity;
    }

    Private DeviceIdentityProfile GetDeviceIdentityProfileEntity(const DeviceIdentityProfileDto& deviceIdentityProfileDto) {
        DeviceIdentityProfile deviceIdentityProfileEntity;
        deviceIdentityProfileEntity.id = 1;
        deviceIdentityProfileEntity.mqttEndpoint = deviceIdentityProfileDto.mqttEndpoint;
        deviceIdentityProfileEntity.caCertificatePem = deviceIdentityProfileDto.caCertificatePem;
        deviceIdentityProfileEntity.clientCertificatePem = deviceIdentityProfileDto.clientCertificatePem;
        deviceIdentityProfileEntity.clientPrivateKeyPem = deviceIdentityProfileDto.clientPrivateKeyPem;
        
        PublishTopics publishTopicsEntity;
        publishTopicsEntity.statusTopic = deviceIdentityProfileDto.publishTopics->statusTopic;
        publishTopicsEntity.telemetryTopic = deviceIdentityProfileDto.publishTopics->telemetryTopic;
        publishTopicsEntity.logsTopic = deviceIdentityProfileDto.publishTopics->logsTopic;
        publishTopicsEntity.eventsTopic = deviceIdentityProfileDto.publishTopics->eventsTopic;
        deviceIdentityProfileEntity.publishTopics = publishTopicsEntity;
        
        SubscribeTopics subscribeTopicsEntity;
        subscribeTopicsEntity.commandTopic = deviceIdentityProfileDto.subscribeTopics->commandTopic;
        subscribeTopicsEntity.otaUpdateTopic = deviceIdentityProfileDto.subscribeTopics->otaUpdateTopic;
        subscribeTopicsEntity.featureFlagTopic = deviceIdentityProfileDto.subscribeTopics->featureFlagTopic;
        deviceIdentityProfileEntity.subscribeTopics = subscribeTopicsEntity;

        return deviceIdentityProfileEntity;
    }
};
#endif // DEVICESERVICE_H