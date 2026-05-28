#ifndef IDEVICESERVICE_H
#define IDEVICESERVICE_H

#include <StandardDefines.h>

#include "../dto/MqttCredentialsDto.h"
#include "../data/FleetProvisioningProfileData.h"

DefineStandardPointers(IDeviceService)
class IDeviceService {
    Public Virtual ~IDeviceService() = default;
    
    Public Virtual StdString GetSerialNumber() const = 0;
    Public Virtual StdString GetDeviceSecret() const = 0;
    Public Virtual StdString GetFirmwareVersion() const = 0;

    Public Virtual FleetProvisioningProfileData GetFleetProvisioningProfile() const = 0;

    // Connection credentials
    Public Virtual Optional<DeviceIdentityProfileData> GetDeviceIdentityProfile() const = 0;

    Public Virtual Void SetFleetProvisioningProfile(const FleetProvisioningProfileDto& fleetProvisioningProfileDto) = 0;
    Public Virtual Void SetDeviceIdentityProfile(const DeviceIdentityProfileDto& deviceIdentityProfileDto) = 0;

    Public Virtual Void Refresh() = 0;
};

#endif // IDEVICESERVICE_H