#ifndef CONNECTIONDETAILSPROVIDER_H
#define CONNECTIONDETAILSPROVIDER_H

#include <StandardDefines.h>
#include "IConnectionDetailsProvider.h"
#include "../repository/FleetProvisioningProfileRepository.h"
#include "../repository/DeviceIdentityProfileRepository.h"

/* @Component */
class ConnectionDetailsProvider final : public IConnectionDetailsProvider {

    Public ConnectionDetailsProvider() {
        Refresh();
    }

    Public StdString GetSerialNumber() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return "1234";
    }

    Public StdString GetDeviceSecret() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return "1234";
    }

    Public StdString GetFirmwareVersion() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return "1.0.0";
    }

    Public StdString GetDeviceType() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceType;
    }

    Public StdString GetTenantId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return tenantId;
    }

    Public StdString GetThingName() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return thingName;
    }

    Public StdString GetFleetProvisioningMqttEndpoint() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningMqttEndpoint;
    }

    Public StdString GetFleetProvisioningTemplateName() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningTemplateName;
    }

    Public StdString GetFleetProvisioningCaCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCaCertificatePem;
    }

    Public StdString GetFleetProvisioningClientCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningClientCertificatePem;
    }

    Public StdString GetFleetProvisioningClientPrivateKeyPem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningClientPrivateKeyPem;
    }

    Public StdString GetFleetProvisioningCreateKeysRequestTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCreateKeysRequestTopic;
    }

    Public StdString GetFleetProvisioningCreateKeysAcceptedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCreateKeysAcceptedTopic;
    }

    Public StdString GetFleetProvisioningCreateKeysRejectedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCreateKeysRejectedTopic;
    }

    Public StdString GetFleetProvisioningProvisionRequestTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningProvisionRequestTopic;
    }

    Public StdString GetFleetProvisioningProvisionAcceptedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningProvisionAcceptedTopic;
    }

    Public StdString GetFleetProvisioningProvisionRejectedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningProvisionRejectedTopic;
    }

    Public Bool IsDeviceIdentityProfilePresent() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return isDeviceIdentityProfilePresent;
    }

    Public StdString GetDeviceIdentityMqttEndpoint() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityMqttEndpoint;
    }

    Public StdString GetDeviceIdentityCaCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityCaCertificatePem;
    }

    Public StdString GetDeviceIdentityClientCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityClientCertificatePem;
    }

    Public StdString GetDeviceIdentityClientPrivateKeyPem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityClientPrivateKeyPem;
    }

    Public StdString GetDeviceIdentityPublishTopicsStatusTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsStatusTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsTelemetryTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsTelemetryTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsLogsTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsLogsTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsEventsTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsEventsTopic;
    }

    Public StdString GetDeviceIdentitySubscribeTopicsCommandTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentitySubscribeTopicsCommandTopic;
    }

    Public StdString GetDeviceIdentitySubscribeTopicsOtaUpdateTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentitySubscribeTopicsOtaUpdateTopic;
    }

    Public StdString GetDeviceIdentitySubscribeTopicsFeatureFlagTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentitySubscribeTopicsFeatureFlagTopic;
    }

    Public Void Refresh() override {
        std::lock_guard<std::mutex> lock(mutex_);
        RefreshFleetProvisioningProfile();
        RefreshDeviceIdentityProfile();
    }

    Private Void RefreshFleetProvisioningProfile() {
        Val fleetProvisioningProfileEntityOpt = fleetProvisioningProfileRepository->FindFirst();
        if (fleetProvisioningProfileEntityOpt.has_value()) {
            Val fleetProvisioningProfileEntity = fleetProvisioningProfileEntityOpt.value();

            if (fleetProvisioningProfileEntity.mqttEndpoint.has_value()) fleetProvisioningMqttEndpoint = fleetProvisioningProfileEntity.mqttEndpoint.value();
            if (fleetProvisioningProfileEntity.templateName.has_value()) fleetProvisioningTemplateName = fleetProvisioningProfileEntity.templateName.value();
            if (fleetProvisioningProfileEntity.caCertificatePem.has_value()) fleetProvisioningCaCertificatePem = fleetProvisioningProfileEntity.caCertificatePem.value();
            if (fleetProvisioningProfileEntity.clientCertificatePem.has_value()) fleetProvisioningClientCertificatePem = fleetProvisioningProfileEntity.clientCertificatePem.value();
            if (fleetProvisioningProfileEntity.clientPrivateKeyPem.has_value()) fleetProvisioningClientPrivateKeyPem = fleetProvisioningProfileEntity.clientPrivateKeyPem.value();

            if (fleetProvisioningProfileEntity.templateName.has_value()) fleetProvisioningTemplateName = fleetProvisioningProfileEntity.templateName.value();

            if (fleetProvisioningProfileEntity.createKeysRequestTopic.has_value()) fleetProvisioningCreateKeysRequestTopic = fleetProvisioningProfileEntity.createKeysRequestTopic.value();
            if (fleetProvisioningProfileEntity.createKeysAcceptedTopic.has_value()) fleetProvisioningCreateKeysAcceptedTopic = fleetProvisioningProfileEntity.createKeysAcceptedTopic.value();
            if (fleetProvisioningProfileEntity.createKeysRejectedTopic.has_value()) fleetProvisioningCreateKeysRejectedTopic = fleetProvisioningProfileEntity.createKeysRejectedTopic.value();

            if (fleetProvisioningProfileEntity.provisionRequestTopic.has_value()) {
                fleetProvisioningProvisionRequestTopic = fleetProvisioningProfileEntity.provisionRequestTopic.value();
            } else {
                if (fleetProvisioningProfileEntity.provisionRequestTopicPrefix.has_value()) fleetProvisioningProvisionRequestTopicPrefix = fleetProvisioningProfileEntity.provisionRequestTopicPrefix.value();
                if (fleetProvisioningProfileEntity.provisionRequestTopicSuffix.has_value()) fleetProvisioningProvisionRequestTopicSuffix = fleetProvisioningProfileEntity.provisionRequestTopicSuffix.value();
                fleetProvisioningProvisionRequestTopic = fleetProvisioningProvisionRequestTopicPrefix + 
                                                        fleetProvisioningTemplateName + 
                                                        fleetProvisioningProvisionRequestTopicSuffix;
            } 

            if (fleetProvisioningProfileEntity.provisionAcceptedTopic.has_value()) {
                fleetProvisioningProvisionAcceptedTopic = fleetProvisioningProfileEntity.provisionAcceptedTopic.value();
            } else {
                if (fleetProvisioningProfileEntity.provisionAcceptedTopicPrefix.has_value()) fleetProvisioningProvisionAcceptedTopicPrefix = fleetProvisioningProfileEntity.provisionAcceptedTopicPrefix.value();
                if (fleetProvisioningProfileEntity.provisionAcceptedTopicSuffix.has_value()) fleetProvisioningProvisionAcceptedTopicSuffix = fleetProvisioningProfileEntity.provisionAcceptedTopicSuffix.value();
                fleetProvisioningProvisionAcceptedTopic = fleetProvisioningProvisionAcceptedTopicPrefix + 
                                                        fleetProvisioningTemplateName + 
                                                        fleetProvisioningProvisionAcceptedTopicSuffix;
            }
            
            if (fleetProvisioningProfileEntity.provisionRejectedTopic.has_value()) {
                fleetProvisioningProvisionRejectedTopic = fleetProvisioningProfileEntity.provisionRejectedTopic.value();
            } else {
                if (fleetProvisioningProfileEntity.provisionRejectedTopicPrefix.has_value()) fleetProvisioningProvisionRejectedTopicPrefix = fleetProvisioningProfileEntity.provisionRejectedTopicPrefix.value();
                if (fleetProvisioningProfileEntity.provisionRejectedTopicSuffix.has_value()) fleetProvisioningProvisionRejectedTopicSuffix = fleetProvisioningProfileEntity.provisionRejectedTopicSuffix.value();
                fleetProvisioningProvisionRejectedTopic = fleetProvisioningProvisionRejectedTopicPrefix + 
                                                        fleetProvisioningTemplateName + 
                                                        fleetProvisioningProvisionRejectedTopicSuffix;
            }
        } 
    }

    Private Void RefreshDeviceIdentityProfile() {
        Val deviceIdentityProfileEntityOpt = deviceIdentityProfileRepository->FindFirst();
        if (deviceIdentityProfileEntityOpt.has_value()) {
            isDeviceIdentityProfilePresent = true;
            Val deviceIdentityProfileEntity = deviceIdentityProfileEntityOpt.value();

            if (deviceIdentityProfileEntity.mqttEndpoint.has_value()) deviceIdentityMqttEndpoint = deviceIdentityProfileEntity.mqttEndpoint.value();

            if (deviceIdentityProfileEntity.tenantId.has_value()) tenantId = deviceIdentityProfileEntity.tenantId.value();
            if (deviceIdentityProfileEntity.deviceType.has_value()) deviceType = deviceIdentityProfileEntity.deviceType.value();
            if (deviceIdentityProfileEntity.thingName.has_value()) thingName = deviceIdentityProfileEntity.thingName.value();

            if (deviceIdentityProfileEntity.caCertificatePem.has_value()) deviceIdentityCaCertificatePem = deviceIdentityProfileEntity.caCertificatePem.value();
            if (deviceIdentityProfileEntity.clientCertificatePem.has_value()) deviceIdentityClientCertificatePem = deviceIdentityProfileEntity.clientCertificatePem.value();
            if (deviceIdentityProfileEntity.clientPrivateKeyPem.has_value()) deviceIdentityClientPrivateKeyPem = deviceIdentityProfileEntity.clientPrivateKeyPem.value();

            StdString deviceIdentityTopicsPrefix = tenantId + "/" + deviceType + "/" + thingName;

            if(deviceIdentityProfileEntity.publishTopics.has_value()) {
                if(deviceIdentityProfileEntity.publishTopics->statusTopic.has_value()) deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + "/status";
                if(deviceIdentityProfileEntity.publishTopics->telemetryTopic.has_value()) deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + "/telemetry";
                if(deviceIdentityProfileEntity.publishTopics->logsTopic.has_value()) deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + "/logs";
                if(deviceIdentityProfileEntity.publishTopics->eventsTopic.has_value()) deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + "/events";
            }

            if(deviceIdentityProfileEntity.subscribeTopics.has_value()) {
                if(deviceIdentityProfileEntity.subscribeTopics->commandTopic.has_value()) deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + "/command";
                if(deviceIdentityProfileEntity.subscribeTopics->otaUpdateTopic.has_value()) deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + "/ota/update";
                if(deviceIdentityProfileEntity.subscribeTopics->featureFlagTopic.has_value()) deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + "/feature/flag";
            }
        } 
    }

    Private mutable std::mutex mutex_;

    Private StdString fleetProvisioningMqttEndpoint = "mqtts://a2hlcpmplecdfa-ats.iot.us-east-1.amazonaws.com";
    Private StdString fleetProvisioningCaCertificatePem = "fleet/provisioning/ca/certificate.pem";
    Private StdString fleetProvisioningClientCertificatePem = "fleet/provisioning/client/certificate.pem";
    Private StdString fleetProvisioningClientPrivateKeyPem = "fleet/provisioning/client/private/key.pem";
    
    Private StdString fleetProvisioningCreateKeysRequestTopic = "$aws/certificates/create/json";
    Private StdString fleetProvisioningCreateKeysAcceptedTopic = "$aws/certificates/create/json/accepted";
    Private StdString fleetProvisioningCreateKeysRejectedTopic = "$aws/certificates/create/json/rejected";

    Private StdString fleetProvisioningTemplateName = "SomeTemplateName";

    Private StdString fleetProvisioningProvisionRequestTopicPrefix = "$aws/provisioning-templates/";
    Private StdString fleetProvisioningProvisionRequestTopicSuffix = "/provision/json";
    Private StdString fleetProvisioningProvisionRequestTopic = fleetProvisioningProvisionRequestTopicPrefix + fleetProvisioningTemplateName + fleetProvisioningProvisionRequestTopicSuffix;

    Private StdString fleetProvisioningProvisionAcceptedTopicPrefix = "$aws/provisioning-templates/";
    Private StdString fleetProvisioningProvisionAcceptedTopicSuffix = "/provision/json/accepted";
    Private StdString fleetProvisioningProvisionAcceptedTopic = fleetProvisioningProvisionAcceptedTopicPrefix + fleetProvisioningTemplateName + fleetProvisioningProvisionAcceptedTopicSuffix;

    Private StdString fleetProvisioningProvisionRejectedTopicPrefix = "$aws/provisioning-templates/";
    Private StdString fleetProvisioningProvisionRejectedTopicSuffix = "/provision/json/rejected";
    Private StdString fleetProvisioningProvisionRejectedTopic = fleetProvisioningProvisionRejectedTopicPrefix + fleetProvisioningTemplateName + fleetProvisioningProvisionRejectedTopicSuffix;

    /* @Autowired */
    Private FleetProvisioningProfileRepositoryPtr fleetProvisioningProfileRepository;

    Private StdString tenantId = "123";
    Private StdString deviceType = "switch";
    Private StdString thingName = "thing";

    Private StdString deviceIdentityTopicsPrefix = tenantId + "/" + deviceType + "/" + thingName;

    Private Bool isDeviceIdentityProfilePresent = false;

    Private StdString deviceIdentityMqttEndpoint;
    Private StdString deviceIdentityCaCertificatePem; 
    Private StdString deviceIdentityClientCertificatePem; 
    Private StdString deviceIdentityClientPrivateKeyPem; 

    Private StdString deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + "/status";
    Private StdString deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + "/telemetry";
    Private StdString deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + "/logs";
    Private StdString deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + "/events";

    Private StdString deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + "/command";
    Private StdString deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + "/ota/update";
    Private StdString deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + "/feature/flag";

    /* @Autowired */
    Private DeviceIdentityProfileRepositoryPtr deviceIdentityProfileRepository;
};


#endif