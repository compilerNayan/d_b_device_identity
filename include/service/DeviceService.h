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

        if(connectionDetailsProvider->IsDeviceIdentityProfilePresent()) {
            this->deviceIdentityProfile->mqttEndpoint = connectionDetailsProvider->GetDeviceIdentityMqttEndpoint();
            this->deviceIdentityProfile->caCertificatePem = connectionDetailsProvider->GetDeviceIdentityCaCertificatePem();
            
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

        fleetProvisioningProfileEntity.mqttEndpoint = fleetProvisioningProfileDto.mqttEndpoint.value();

        fleetProvisioningProfileEntity.caCertificatePem = fleetProvisioningProfileDto.caCertificatePem.value();
        fleetProvisioningProfileEntity.clientCertificatePem = fleetProvisioningProfileDto.clientCertificatePem.value();
        fleetProvisioningProfileEntity.clientPrivateKeyPem = fleetProvisioningProfileDto.clientPrivateKeyPem.value();

        fleetProvisioningProfileEntity.templateName = fleetProvisioningProfileDto.templateName.value();

        fleetProvisioningProfileEntity.createKeysRequestTopic = fleetProvisioningProfileDto.createKeysRequestTopic.value();
        fleetProvisioningProfileEntity.createKeysAcceptedTopic = fleetProvisioningProfileDto.createKeysAcceptedTopic.value();
        fleetProvisioningProfileEntity.createKeysRejectedTopic = fleetProvisioningProfileDto.createKeysRejectedTopic.value();

        fleetProvisioningProfileEntity.provisionRequestTopicPrefix = fleetProvisioningProfileDto.provisionRequestTopicPrefix.value();
        fleetProvisioningProfileEntity.provisionRequestTopicSuffix = fleetProvisioningProfileDto.provisionRequestTopicSuffix.value();
        fleetProvisioningProfileEntity.provisionRequestTopic = fleetProvisioningProfileDto.provisionRequestTopic.value();

        fleetProvisioningProfileEntity.provisionAcceptedTopicPrefix = fleetProvisioningProfileDto.provisionAcceptedTopicPrefix.value();
        fleetProvisioningProfileEntity.provisionAcceptedTopicSuffix = fleetProvisioningProfileDto.provisionAcceptedTopicSuffix.value();
        fleetProvisioningProfileEntity.provisionAcceptedTopic = fleetProvisioningProfileDto.provisionAcceptedTopic.value();

        fleetProvisioningProfileEntity.provisionRejectedTopicPrefix = fleetProvisioningProfileDto.provisionRejectedTopicPrefix.value();
        fleetProvisioningProfileEntity.provisionRejectedTopicSuffix = fleetProvisioningProfileDto.provisionRejectedTopicSuffix.value();
        fleetProvisioningProfileEntity.provisionRejectedTopic = fleetProvisioningProfileDto.provisionRejectedTopic.value();
        return fleetProvisioningProfileEntity;
    }

    Private DeviceIdentityProfile GetDeviceIdentityProfileEntity(const DeviceIdentityProfileDto& deviceIdentityProfileDto) {
        DeviceIdentityProfile deviceIdentityProfileEntity;

        deviceIdentityProfileEntity.id = 1;

        deviceIdentityProfileEntity.mqttEndpoint = deviceIdentityProfileDto.mqttEndpoint.value();

        deviceIdentityProfileEntity.tenantId = deviceIdentityProfileDto.tenantId.value();
        deviceIdentityProfileEntity.deviceType = deviceIdentityProfileDto.deviceType.value();
        deviceIdentityProfileEntity.thingName = deviceIdentityProfileDto.thingName.value();

        deviceIdentityProfileEntity.caCertificatePem = deviceIdentityProfileDto.caCertificatePem;
        deviceIdentityProfileEntity.clientCertificatePem = deviceIdentityProfileDto.clientCertificatePem;
        deviceIdentityProfileEntity.clientPrivateKeyPem = deviceIdentityProfileDto.clientPrivateKeyPem;
        
        if(deviceIdentityProfileDto.publishTopics.has_value()) {
            PublishTopics publishTopicsEntity;
            publishTopicsEntity.statusTopic = deviceIdentityProfileDto.publishTopics->statusTopic;
            publishTopicsEntity.telemetryTopic = deviceIdentityProfileDto.publishTopics->telemetryTopic;
            publishTopicsEntity.logsTopic = deviceIdentityProfileDto.publishTopics->logsTopic;
            publishTopicsEntity.eventsTopic = deviceIdentityProfileDto.publishTopics->eventsTopic;
            deviceIdentityProfileEntity.publishTopics = publishTopicsEntity;
        }

        if(deviceIdentityProfileDto.subscribeTopics.has_value()) {
            SubscribeTopics subscribeTopicsEntity;
            subscribeTopicsEntity.commandTopic = deviceIdentityProfileDto.subscribeTopics->commandTopic;
            subscribeTopicsEntity.otaUpdateTopic = deviceIdentityProfileDto.subscribeTopics->otaUpdateTopic;
            subscribeTopicsEntity.featureFlagTopic = deviceIdentityProfileDto.subscribeTopics->featureFlagTopic;
            deviceIdentityProfileEntity.subscribeTopics = subscribeTopicsEntity;
        }
        return deviceIdentityProfileEntity;
    }
};
#endif // DEVICESERVICE_H