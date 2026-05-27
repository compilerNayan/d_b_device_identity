#ifndef IDEVICECONFIGPROVIDER_H
#define IDEVICECONFIGPROVIDER_H

#include "../service/IDeviceService.h"

DefineStandardPointers(IDeviceConfigProvider)
class IDeviceConfigProvider {

    Public Virtual ~IDeviceConfigProvider() = default;

    Public Virtual StdString GetSerialNumber() const = 0;
    Public Virtual StdString GetDeviceSecret() const = 0;
    Public Virtual StdString GetFirmwareVersion() const = 0;

    // Enrollment credentials
    Public Virtual MqttCredentialsDto GetEnrollmentCredentials() const = 0;

    // Connection credentials
    Public Virtual Optional<MqttCredentialsDto> GetConnectionCredentials() const = 0;

    Public Virtual StdSet<StdString> GetSubscribeTopics() const = 0;
    
    // Subscribe topics
    Public Virtual StdString GetCommandTopic() const = 0;
    Public Virtual StdString GetOtaUpdateTopic() const = 0;
    Public Virtual StdString GetFeatureFlagTopic() const = 0;

    // Publish topics
    Public Virtual StdString GetStatusTopic() const = 0;
    Public Virtual StdString GetTelemetryTopic() const = 0;
    Public Virtual StdString GetLogsTopic() const = 0;
    Public Virtual StdString GetEventsTopic() const = 0;
};

#endif // IDEVICECONFIGPROVIDER_H