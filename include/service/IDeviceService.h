#ifndef IDEVICESERVICE_H
#define IDEVICESERVICE_H

#include <StandardDefines.h>

#include "../entity/MqttCredentials.h"

DefineStandardPointers(IDeviceService)
class IDeviceService {
    Public Virtual ~IDeviceService() = default;
    
    Public Virtual StdString GetSerialNumber() const = 0;
    Public Virtual StdString GetDeviceSecret() const = 0;
    Public Virtual StdString GetFirmwareVersion() const = 0;

    Public Virtual StdString GetMqttEndpoint() const = 0;

    // Enrollment credentials
    Public Virtual MqttCredentials GetEnrollmentCredentials() const = 0;

    // Connection credentials
    Public Virtual Optional<MqttCredentials> GetConnectionCredentials() const = 0;

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

    Public Virtual Void Refresh() = 0;

    Public Virtual Void SetEnrollmentCredentials(const MqttCredentials& enrollmentCredentials) = 0;
    Public Virtual Void SetConnectionCredentials(const MqttCredentials& connectionCredentials) = 0;
};

#endif // IDEVICESERVICE_H