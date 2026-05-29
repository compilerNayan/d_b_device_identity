#ifndef CONNECTIONDETAILSPROVIDER_H
#define CONNECTIONDETAILSPROVIDER_H

#include <StandardDefines.h>
#include "IConnectionDetailsProvider.h"

/* @Component */
class ConnectionDetailsProvider final : public IConnectionDetailsProvider {

    Public ConnectionDetailsProvider() {}

    Public StdString GetSerialNumber() const override {
        return "1234";
    }

    Public StdString GetDeviceType() const override {
        return deviceType;
    }

    Public StdString GetTenantId() const override {
        return tenantId;
    }

    Public StdString GetThingName() const override {
        return thingName;
    }

    Public StdString GetFleetProvisioningMqttEndpoint() const override {
        return fleetProvisioningMqttEndpoint;
    }

    Public StdString GetFleetProvisioningTemplateName() const override {
        return fleetProvisioningTemplateName;
    }

    Public StdString GetFleetProvisioningCaCertificatePem() const override {
        return fleetProvisioningCaCertificatePem;
    }

    Public StdString GetFleetProvisioningClientCertificatePem() const override {
        return fleetProvisioningClientCertificatePem;
    }

    Public StdString GetFleetProvisioningClientPrivateKeyPem() const override {
        return fleetProvisioningClientPrivateKeyPem;
    }

    Public StdString GetFleetProvisioningCreateKeysRequestTopic() const override {
        return fleetProvisioningCreateKeysRequestTopic;
    }

    Public StdString GetFleetProvisioningCreateKeysAcceptedTopic() const override {
        return fleetProvisioningCreateKeysAcceptedTopic;
    }

    Public StdString GetFleetProvisioningCreateKeysRejectedTopic() const override {
        return fleetProvisioningCreateKeysRejectedTopic;
    }

    Public StdString GetFleetProvisioningProvisionRequestTopic() const override {
        return fleetProvisioningProvisionRequestTopic;
    }

    Public StdString GetFleetProvisioningProvisionAcceptedTopic() const override {
        return fleetProvisioningProvisionAcceptedTopic;
    }

    Public StdString GetFleetProvisioningProvisionRejectedTopic() const override {
        return fleetProvisioningProvisionRejectedTopic;
    }

    Public StdString GetDeviceIdentityMqttEndpoint() const override {
        return deviceIdentityMqttEndpoint;
    }

    Public StdString GetDeviceIdentityCaCertificatePem() const override {
        return deviceIdentityCaCertificatePem;
    }

    Public StdString GetDeviceIdentityClientCertificatePem() const override {
        return deviceIdentityClientCertificatePem;
    }

    Public StdString GetDeviceIdentityClientPrivateKeyPem() const override {
        return deviceIdentityClientPrivateKeyPem;
    }

    Public StdString GetDeviceIdentityPublishTopicsStatusTopic() const override {
        return deviceIdentityPublishTopicsStatusTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsTelemetryTopic() const override {
        return deviceIdentityPublishTopicsTelemetryTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsLogsTopic() const override {
        return deviceIdentityPublishTopicsLogsTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsEventsTopic() const override {
        return deviceIdentityPublishTopicsEventsTopic;
    }

    Public StdString GetDeviceIdentitySubscribeTopicsCommandTopic() const override {
        return deviceIdentitySubscribeTopicsCommandTopic;
    }

    Public Void Refresh() override {
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
            
            if (fleetProvisioningProfileEntity.provisionRejectedTopic.has_value()) fleetProvisioningProvisionRejectedTopic = fleetProvisioningProfileEntity.provisionRejectedTopic.value();
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
            Val deviceIdentityProfileEntity = deviceIdentityProfileEntityOpt.value();

            if (deviceIdentityProfileEntity.mqttEndpoint.has_value()) deviceIdentityMqttEndpoint = deviceIdentityProfileEntity.mqttEndpoint.value();

            if (deviceIdentityProfileEntity.tenantId.has_value()) tenantId = deviceIdentityProfileEntity.tenantId.value();
            if (deviceIdentityProfileEntity.deviceType.has_value()) deviceType = deviceIdentityProfileEntity.deviceType.value();
            if (deviceIdentityProfileEntity.thingName.has_value()) thingName = deviceIdentityProfileEntity.thingName.value();

            if (deviceIdentityProfileEntity.caCertificatePem.has_value()) deviceIdentityCaCertificatePem = deviceIdentityProfileEntity.caCertificatePem.value();
            if (deviceIdentityProfileEntity.clientCertificatePem.has_value()) deviceIdentityClientCertificatePem = deviceIdentityProfileEntity.clientCertificatePem.value();
            if (deviceIdentityProfileEntity.clientPrivateKeyPem.has_value()) deviceIdentityClientPrivateKeyPem = deviceIdentityProfileEntity.clientPrivateKeyPem.value();

            StdString deviceIdentityTopicsPrefix = tenantId + "/" + deviceType + "/" + thingName;

            if (deviceIdentityProfileEntity.publishTopicsStatusTopic.has_value()) deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + "/status";
            if (deviceIdentityProfileEntity.publishTopicsTelemetryTopic.has_value()) deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + "/telemetry";
            if (deviceIdentityProfileEntity.publishTopicsLogsTopic.has_value()) deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + "/logs";
            if (deviceIdentityProfileEntity.publishTopicsEventsTopic.has_value()) deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + "/events";

            if (deviceIdentityProfileEntity.subscribeTopicsCommandTopic.has_value()) deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + "/command";
            if (deviceIdentityProfileEntity.subscribeTopicsOtaUpdateTopic.has_value()) deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + "/ota/update";
            if (deviceIdentityProfileEntity.subscribeTopicsFeatureFlagTopic.has_value()) deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + "/feature/flag";
        }
    }

    Private StdString fleetProvisioningMqttEndpoint = "mqtts://a2hlcpmplecdfa-ats.iot.us-east-1.amazonaws.com";
    Private StdString fleetProvisioningTemplateName = "SomeTemplateName";
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

    Private StdString deviceIdentityMqttEndpoint = "mqtts://a2hlcpmplecdfa-ats.iot.us-east-1.amazonaws.com";
    Private StdString deviceIdentityCaCertificatePem = "device/identity/ca/certificate.pem";
    Private StdString deviceIdentityClientCertificatePem = "device/identity/client/certificate.pem";
    Private StdString deviceIdentityClientPrivateKeyPem = "device/identity/client/private/key.pem";

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