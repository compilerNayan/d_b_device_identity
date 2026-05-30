#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "IDeviceService.h"
#include "../repository/FleetProvisioningProfileRepository.h"
#include "../repository/DeviceIdentityProfileRepository.h"
#include "IConnectionDetailsProvider.h"

/* @Service */
class DeviceService : public IDeviceService {
    Public DeviceService() {
        Refresh();
    }
    Public Virtual ~DeviceService() = default;

    /* @Autowired */
    Private IConnectionDetailsProviderPtr connectionDetailsProvider;

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
        std::lock_guard<std::mutex> lock(mutex_);
        serialNumber = connectionDetailsProvider->GetSerialNumber();
        deviceSecret = connectionDetailsProvider->GetDeviceSecret();
        firmwareVersion = connectionDetailsProvider->GetFirmwareVersion();

        this->fleetProvisioningProfile.mqttEndpoint = connectionDetailsProvider->GetFleetProvisioningMqttEndpoint();
        this->fleetProvisioningProfile.caCertificatePem = connectionDetailsProvider->GetFleetProvisioningCaCertificatePem();
        this->fleetProvisioningProfile.clientCertificatePem = connectionDetailsProvider->GetFleetProvisioningClientCertificatePem();
        this->fleetProvisioningProfile.clientPrivateKeyPem = connectionDetailsProvider->GetFleetProvisioningClientPrivateKeyPem();
        this->fleetProvisioningProfile.createKeysRequestTopic = connectionDetailsProvider->GetFleetProvisioningCreateKeysRequestTopic();
        this->fleetProvisioningProfile.createKeysAcceptedTopic = connectionDetailsProvider->GetFleetProvisioningCreateKeysAcceptedTopic();
        this->fleetProvisioningProfile.createKeysRejectedTopic = connectionDetailsProvider->GetFleetProvisioningCreateKeysRejectedTopic();
        this->fleetProvisioningProfile.provisionRequestTopic = connectionDetailsProvider->GetFleetProvisioningProvisionRequestTopic();
        this->fleetProvisioningProfile.provisionAcceptedTopic = connectionDetailsProvider->GetFleetProvisioningProvisionAcceptedTopic();
        this->fleetProvisioningProfile.provisionRejectedTopic = connectionDetailsProvider->GetFleetProvisioningProvisionRejectedTopic();

        if(connectionDetailsProvider->IsDeviceIdentityProfilePresent()) {
            this->deviceIdentityProfile = std::make_optional<DeviceIdentityProfileData>();
            this->deviceIdentityProfile->mqttEndpoint = connectionDetailsProvider->GetDeviceIdentityMqttEndpoint();
            this->deviceIdentityProfile->caCertificatePem = connectionDetailsProvider->GetDeviceIdentityCaCertificatePem();
            this->deviceIdentityProfile->clientCertificatePem = connectionDetailsProvider->GetDeviceIdentityClientCertificatePem();
            this->deviceIdentityProfile->clientPrivateKeyPem = connectionDetailsProvider->GetDeviceIdentityClientPrivateKeyPem();

            this->deviceIdentityProfile->publishTopics.statusTopic = connectionDetailsProvider->GetDeviceIdentityPublishTopicsStatusTopic();
            this->deviceIdentityProfile->publishTopics.telemetryTopic = connectionDetailsProvider->GetDeviceIdentityPublishTopicsTelemetryTopic();
            this->deviceIdentityProfile->publishTopics.logsTopic = connectionDetailsProvider->GetDeviceIdentityPublishTopicsLogsTopic();
            this->deviceIdentityProfile->publishTopics.eventsTopic = connectionDetailsProvider->GetDeviceIdentityPublishTopicsEventsTopic();
    
            this->deviceIdentityProfile->subscribeTopics.commandTopic = connectionDetailsProvider->GetDeviceIdentitySubscribeTopicsCommandTopic();
            this->deviceIdentityProfile->subscribeTopics.otaUpdateTopic = connectionDetailsProvider->GetDeviceIdentitySubscribeTopicsOtaUpdateTopic();
            this->deviceIdentityProfile->subscribeTopics.featureFlagTopic = connectionDetailsProvider->GetDeviceIdentitySubscribeTopicsFeatureFlagTopic();
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
        connectionDetailsProvider->Refresh();
        Refresh();
    }
    
    Public Void SetDeviceIdentityProfile(const DeviceIdentityProfileDto& deviceIdentityProfileDto) override { 
        Var deviceIdentityProfileEntity = GetDeviceIdentityProfileEntity(deviceIdentityProfileDto);
        deviceIdentityProfileRepository->UpdateAvailableFields(deviceIdentityProfileEntity);
        connectionDetailsProvider->Refresh();
        Refresh();
    }

    Private FleetProvisioningProfile GetFleetProvisioningProfileEntity(const FleetProvisioningProfileDto& fleetProvisioningProfileDto) {
        FleetProvisioningProfile fleetProvisioningProfileEntity;
        fleetProvisioningProfileEntity.id = 1;

        fleetProvisioningProfileEntity.mqttEndpoint = fleetProvisioningProfileDto.mqttEndpoint;

        fleetProvisioningProfileEntity.caCertificatePem = fleetProvisioningProfileDto.caCertificatePem;
        fleetProvisioningProfileEntity.clientCertificatePem = fleetProvisioningProfileDto.clientCertificatePem;
        fleetProvisioningProfileEntity.clientPrivateKeyPem = fleetProvisioningProfileDto.clientPrivateKeyPem;

        fleetProvisioningProfileEntity.templateName = fleetProvisioningProfileDto.templateName;

        fleetProvisioningProfileEntity.createKeysRequestTopic = fleetProvisioningProfileDto.createKeysRequestTopic;
        fleetProvisioningProfileEntity.createKeysAcceptedTopic = fleetProvisioningProfileDto.createKeysAcceptedTopic;
        fleetProvisioningProfileEntity.createKeysRejectedTopic = fleetProvisioningProfileDto.createKeysRejectedTopic;

        fleetProvisioningProfileEntity.provisionRequestTopicPrefix = fleetProvisioningProfileDto.provisionRequestTopicPrefix;
        fleetProvisioningProfileEntity.provisionRequestTopicSuffix = fleetProvisioningProfileDto.provisionRequestTopicSuffix;
        fleetProvisioningProfileEntity.provisionRequestTopic = fleetProvisioningProfileDto.provisionRequestTopic;

        fleetProvisioningProfileEntity.provisionAcceptedTopicPrefix = fleetProvisioningProfileDto.provisionAcceptedTopicPrefix;
        fleetProvisioningProfileEntity.provisionAcceptedTopicSuffix = fleetProvisioningProfileDto.provisionAcceptedTopicSuffix;
        fleetProvisioningProfileEntity.provisionAcceptedTopic = fleetProvisioningProfileDto.provisionAcceptedTopic;

        fleetProvisioningProfileEntity.provisionRejectedTopicPrefix = fleetProvisioningProfileDto.provisionRejectedTopicPrefix;
        fleetProvisioningProfileEntity.provisionRejectedTopicSuffix = fleetProvisioningProfileDto.provisionRejectedTopicSuffix;
        fleetProvisioningProfileEntity.provisionRejectedTopic = fleetProvisioningProfileDto.provisionRejectedTopic;
        return fleetProvisioningProfileEntity;
    }

    Private DeviceIdentityProfile GetDeviceIdentityProfileEntity(const DeviceIdentityProfileDto& deviceIdentityProfileDto) {
        DeviceIdentityProfile deviceIdentityProfileEntity;

        deviceIdentityProfileEntity.id = 1;

        deviceIdentityProfileEntity.mqttEndpoint = deviceIdentityProfileDto.mqttEndpoint;

        deviceIdentityProfileEntity.tenantId = deviceIdentityProfileDto.tenantId;
        deviceIdentityProfileEntity.deviceType = deviceIdentityProfileDto.deviceType;
        deviceIdentityProfileEntity.thingName = deviceIdentityProfileDto.thingName;

        deviceIdentityProfileEntity.caCertificatePem = deviceIdentityProfileDto.caCertificatePem;
        deviceIdentityProfileEntity.clientCertificatePem = deviceIdentityProfileDto.clientCertificatePem;
        deviceIdentityProfileEntity.clientPrivateKeyPem = deviceIdentityProfileDto.clientPrivateKeyPem;
        
        if(deviceIdentityProfileDto.publishTopics.has_value()) {
            PublishTopics publishTopicsEntity;
            publishTopicsEntity.statusTopic = deviceIdentityProfileDto.publishTopics->statusTopic;
            publishTopicsEntity.telemetryTopic = deviceIdentityProfileDto.publishTopics->telemetryTopic;
            publishTopicsEntity.logsTopic = deviceIdentityProfileDto.publishTopics->logsTopic;
            publishTopicsEntity.eventsTopic = deviceIdentityProfileDto.publishTopics->eventsTopic;
            if(publishTopicsEntity.statusTopic.has_value() ||
                publishTopicsEntity.telemetryTopic.has_value() ||
                publishTopicsEntity.logsTopic.has_value() ||
                publishTopicsEntity.eventsTopic.has_value()) {
                    deviceIdentityProfileEntity.publishTopics = publishTopicsEntity;
            }
        }

        if(deviceIdentityProfileDto.subscribeTopics.has_value()) {
            SubscribeTopics subscribeTopicsEntity;
            subscribeTopicsEntity.commandTopic = deviceIdentityProfileDto.subscribeTopics->commandTopic;
            subscribeTopicsEntity.otaUpdateTopic = deviceIdentityProfileDto.subscribeTopics->otaUpdateTopic;
            subscribeTopicsEntity.featureFlagTopic = deviceIdentityProfileDto.subscribeTopics->featureFlagTopic;
            if(subscribeTopicsEntity.commandTopic.has_value() ||
                subscribeTopicsEntity.otaUpdateTopic.has_value() ||
                subscribeTopicsEntity.featureFlagTopic.has_value()) {
                    deviceIdentityProfileEntity.subscribeTopics = subscribeTopicsEntity;
            }
        }
        return deviceIdentityProfileEntity;
    }
};
#endif // DEVICESERVICE_H