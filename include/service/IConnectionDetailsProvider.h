#ifndef ICONNECTIONDETAILSPROVIDER_H
#define ICONNECTIONDETAILSPROVIDER_H

#include <StandardDefines.h>

DefineStandardPointers(IConnectionDetailsProvider);
class IConnectionDetailsProvider {
    Public Virtual ~IConnectionDetailsProvider() = default;

    Public Virtual StdString GetSerialNumber() const = 0;
    Public Virtual StdString GetDeviceType() const = 0;
    Public Virtual StdString GetTenantId() const = 0;
    Public Virtual StdString GetThingName() const = 0;

    Public Virtual StdString GetFleetProvisioningMqttEndpoint() const = 0;
    Public Virtual StdString GetFleetProvisioningTemplateName() const = 0;
    Public Virtual StdString GetFleetProvisioningCaCertificatePem() const = 0;
    Public Virtual StdString GetFleetProvisioningClientCertificatePem() const = 0;
    Public Virtual StdString GetFleetProvisioningClientPrivateKeyPem() const = 0;

    Public Virtual StdString GetFleetProvisioningCreateKeysRequestTopic() const = 0;
    Public Virtual StdString GetFleetProvisioningCreateKeysAcceptedTopic() const = 0;
    Public Virtual StdString GetFleetProvisioningCreateKeysRejectedTopic() const = 0;

    Public Virtual StdString GetFleetProvisioningProvisionRequestTopic() const = 0;
    Public Virtual StdString GetFleetProvisioningProvisionAcceptedTopic() const = 0;
    Public Virtual StdString GetFleetProvisioningProvisionRejectedTopic() const = 0;

    Public Virtual StdString GetDeviceIdentityMqttEndpoint() const = 0;
    Public Virtual StdString GetDeviceIdentityCaCertificatePem() const = 0;
    Public Virtual StdString GetDeviceIdentityClientCertificatePem() const = 0;
    Public Virtual StdString GetDeviceIdentityClientPrivateKeyPem() const = 0;

    Public Virtual StdString GetDeviceIdentityPublishTopicsStatusTopic() const = 0;
    Public Virtual StdString GetDeviceIdentityPublishTopicsTelemetryTopic() const = 0;
    Public Virtual StdString GetDeviceIdentityPublishTopicsLogsTopic() const = 0;
    Public Virtual StdString GetDeviceIdentityPublishTopicsEventsTopic() const = 0;

    Public Virtual StdString GetDeviceIdentitySubscribeTopicsCommandTopic() const = 0;
    Public Virtual StdString GetDeviceIdentitySubscribeTopicsOtaUpdateTopic() const = 0;
    Public Virtual StdString GetDeviceIdentitySubscribeTopicsFeatureFlagTopic() const = 0;

    Public Virtual Void Refresh() = 0;

};

#endif // ICONNECTIONDETAILSPROVIDER_H