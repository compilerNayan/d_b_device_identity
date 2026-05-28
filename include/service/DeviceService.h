#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "IDeviceService.h"
#include "../repository/FleetProvisioningProfileRepository.h"
#include "../repository/DeviceIdentityProfileRepository.h"

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

        if (fleetProvisioningProfileOpt.has_value()) {
            Val fleetProvisioningProfileEntity = fleetProvisioningProfileOpt.value();
            this->fleetProvisioningProfile.mqttEndpoint = fleetProvisioningProfileEntity.mqttEndpoint.has_value() ? fleetProvisioningProfileEntity.mqttEndpoint.value() : "";
            this->fleetProvisioningProfile.caCertificatePem = fleetProvisioningProfileEntity.caCertificatePem.has_value() ? fleetProvisioningProfileEntity.caCertificatePem.value() : "";
            this->fleetProvisioningProfile.clientCertificatePem = fleetProvisioningProfileEntity.clientCertificatePem.has_value() ? fleetProvisioningProfileEntity.clientCertificatePem.value() : "";
            this->fleetProvisioningProfile.clientPrivateKeyPem = fleetProvisioningProfileEntity.clientPrivateKeyPem.has_value() ? fleetProvisioningProfileEntity.clientPrivateKeyPem.value() : "";
            this->fleetProvisioningProfile.createKeysRequestTopic = fleetProvisioningProfileEntity.createKeysRequestTopic.has_value() ? fleetProvisioningProfileEntity.createKeysRequestTopic.value() : "";
            this->fleetProvisioningProfile.createKeysAcceptedTopic = fleetProvisioningProfileEntity.createKeysAcceptedTopic.has_value() ? fleetProvisioningProfileEntity.createKeysAcceptedTopic.value() : "";
            this->fleetProvisioningProfile.createKeysRejectedTopic = fleetProvisioningProfileEntity.createKeysRejectedTopic.has_value() ? fleetProvisioningProfileEntity.createKeysRejectedTopic.value() : "";
            this->fleetProvisioningProfile.provisionRequestTopic = fleetProvisioningProfileEntity.provisionRequestTopic.has_value() ? fleetProvisioningProfileEntity.provisionRequestTopic.value() : "";
            this->fleetProvisioningProfile.provisionAcceptedTopic = fleetProvisioningProfileEntity.provisionAcceptedTopic.has_value() ? fleetProvisioningProfileEntity.provisionAcceptedTopic.value() : "";
        } else {
            this->fleetProvisioningProfile.mqttEndpoint = "";
            this->fleetProvisioningProfile.caCertificatePem = "";
            this->fleetProvisioningProfile.clientCertificatePem = "";
            this->fleetProvisioningProfile.clientPrivateKeyPem = "";
            this->fleetProvisioningProfile.createKeysRequestTopic = "";
            this->fleetProvisioningProfile.createKeysAcceptedTopic = "";
            this->fleetProvisioningProfile.createKeysRejectedTopic = "";
            this->fleetProvisioningProfile.provisionRequestTopic = "";
            this->fleetProvisioningProfile.provisionAcceptedTopic = "";
        }

        if(deviceIdentityProfileOpt.has_value()) {
            Val deviceIdentityProfileEntity = deviceIdentityProfileOpt.value();
            this->deviceIdentityProfile.mqttEndpoint = deviceIdentityProfileEntity.mqttEndpoint.has_value() ? deviceIdentityProfileEntity.mqttEndpoint.value() : "";
            this->deviceIdentityProfile.caCertificatePem = deviceIdentityProfileEntity.caCertificatePem.has_value() ? deviceIdentityProfileEntity.caCertificatePem.value() : "";
            this->deviceIdentityProfile.clientCertificatePem = deviceIdentityProfileEntity.clientCertificatePem.has_value() ? deviceIdentityProfileEntity.clientCertificatePem.value() : "";
            this->deviceIdentityProfile.clientPrivateKeyPem = deviceIdentityProfileEntity.clientPrivateKeyPem.has_value() ? deviceIdentityProfileEntity.clientPrivateKeyPem.value() : "";
            
            this->deviceIdentityProfile.publishTopics.statusTopic = "";
            this->deviceIdentityProfile.publishTopics.telemetryTopic = "";
            this->deviceIdentityProfile.publishTopics.logsTopic = "";
            this->deviceIdentityProfile.publishTopics.eventsTopic = "";

            if(deviceIdentityProfileEntity.publishTopics.has_value()) {
                Val publishTopicsEntity = deviceIdentityProfileEntity.publishTopics.value();
                if(publishTopicsEntity.statusTopic.has_value()) this->deviceIdentityProfile.publishTopics.statusTopic = publishTopicsEntity.statusTopic.value();
                if(publishTopicsEntity.telemetryTopic.has_value()) this->deviceIdentityProfile.publishTopics.telemetryTopic = publishTopicsEntity.telemetryTopic.value();
                if(publishTopicsEntity.logsTopic.has_value()) this->deviceIdentityProfile.publishTopics.logsTopic = publishTopicsEntity.logsTopic.value();
                if(publishTopicsEntity.eventsTopic.has_value()) this->deviceIdentityProfile.publishTopics.eventsTopic = publishTopicsEntity.eventsTopic.value();
            }

            this->deviceIdentityProfile.subscribeTopics.commandTopic = "";
            this->deviceIdentityProfile.subscribeTopics.otaUpdateTopic = "";
            this->deviceIdentityProfile.subscribeTopics.featureFlagTopic = "";

            if(deviceIdentityProfileEntity.subscribeTopics.has_value()) {
                Val subscribeTopicsEntity = deviceIdentityProfileEntity.subscribeTopics.value();
                if(subscribeTopicsEntity.commandTopic.has_value()) this->deviceIdentityProfile.subscribeTopics.commandTopic = subscribeTopicsEntity.commandTopic.value();
                if(subscribeTopicsEntity.otaUpdateTopic.has_value()) this->deviceIdentityProfile.subscribeTopics.otaUpdateTopic = subscribeTopicsEntity.otaUpdateTopic.value();
                if(subscribeTopicsEntity.featureFlagTopic.has_value()) this->deviceIdentityProfile.subscribeTopics.featureFlagTopic = subscribeTopicsEntity.featureFlagTopic.value();
            }
        } else {
            this->deviceIdentityProfile = std::nullopt;
        }

        if (publishTopicsOpt.has_value()) {
            Val publishTopics = publishTopicsOpt.value();
            this->statusTopic = publishTopics.statusTopic.has_value() ? publishTopics.statusTopic.value() : "";
            this->telemetryTopic = publishTopics.telemetryTopic.has_value() ? publishTopics.telemetryTopic.value() : "";
            this->logsTopic = publishTopics.logsTopic.has_value() ? publishTopics.logsTopic.value() : "";
            this->eventsTopic = publishTopics.eventsTopic.has_value() ? publishTopics.eventsTopic.value() : "";
        }

        if (subscribeTopicsOpt.has_value()) {
            Val subscribeTopics = subscribeTopicsOpt.value();
            this->commandTopic = subscribeTopics.commandTopic.has_value() ? subscribeTopics.commandTopic.value() : "";
            this->otaUpdateTopic = subscribeTopics.otaUpdateTopic.has_value() ? subscribeTopics.otaUpdateTopic.value() : "";
            this->featureFlagTopic = subscribeTopics.featureFlagTopic.has_value() ? subscribeTopics.featureFlagTopic.value() : "";
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
        fleetProvisioningProfileRepository->UpdateAvailableFields(fleetProvisioningProfileDto);
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
        fleetProvisioningProfileEntity.provisionRejectedTopic = fleetProvisioningProfileDto.provisionRejectedTopic.value();
        return fleetProvisioningProfileEntity;
    }

    Private DeviceIdentityProfile GetDeviceIdentityProfileEntity(const DeviceIdentityProfileDto& deviceIdentityProfileDto) {
        DeviceIdentityProfile deviceIdentityProfileEntity;
        deviceIdentityProfileEntity.id = 1;
        deviceIdentityProfileEntity.mqttEndpoint = deviceIdentityProfileDto.mqttEndpoint.value();
        deviceIdentityProfileEntity.caCertificatePem = deviceIdentityProfileDto.caCertificatePem.value();
        deviceIdentityProfileEntity.clientCertificatePem = deviceIdentityProfileDto.clientCertificatePem.value();
        deviceIdentityProfileEntity.clientPrivateKeyPem = deviceIdentityProfileDto.clientPrivateKeyPem.value();
        deviceIdentityProfileEntity.publishTopics.statusTopic = deviceIdentityProfileDto.publishTopics.statusTopic.value();
        deviceIdentityProfileEntity.publishTopics.telemetryTopic = deviceIdentityProfileDto.publishTopics.telemetryTopic.value();
        deviceIdentityProfileEntity.publishTopics.logsTopic = deviceIdentityProfileDto.publishTopics.logsTopic.value();
        deviceIdentityProfileEntity.publishTopics.eventsTopic = deviceIdentityProfileDto.publishTopics.eventsTopic.value();
        deviceIdentityProfileEntity.subscribeTopics.commandTopic = deviceIdentityProfileDto.subscribeTopics.commandTopic.value();
        deviceIdentityProfileEntity.subscribeTopics.otaUpdateTopic = deviceIdentityProfileDto.subscribeTopics.otaUpdateTopic.value();
        deviceIdentityProfileEntity.subscribeTopics.featureFlagTopic = deviceIdentityProfileDto.subscribeTopics.featureFlagTopic.value();
        return deviceIdentityProfileEntity;
    }
};
#endif // DEVICESERVICE_H