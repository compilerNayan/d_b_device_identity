#ifndef DEVICEIDENTITYPROFILEREPOSITORY_H
#define DEVICEIDENTITYPROFILEREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/DeviceIdentityProfile.h"

/* @Repository */
DefineStandardPointers(DeviceIdentityProfileRepository)
class DeviceIdentityProfileRepository : public CpaRepository<DeviceIdentityProfile, int> {
    Public Virtual ~DeviceIdentityProfileRepository() = default;

    Public DeviceIdentityProfile UpdateAvailableFields(DeviceIdentityProfile& deviceIdentityProfile) {
        Var existingEntity = FindFirst();
        if(existingEntity.has_value()) {
            if(deviceIdentityProfile.mqttEndpoint.has_value()) existingEntity.value().mqttEndpoint = deviceIdentityProfile.mqttEndpoint.value();
            if(deviceIdentityProfile.caCertificatePem.has_value()) existingEntity.value().caCertificatePem = deviceIdentityProfile.caCertificatePem.value();
            if(deviceIdentityProfile.clientCertificatePem.has_value()) existingEntity.value().clientCertificatePem = deviceIdentityProfile.clientCertificatePem.value();
            if(deviceIdentityProfile.clientPrivateKeyPem.has_value()) existingEntity.value().clientPrivateKeyPem = deviceIdentityProfile.clientPrivateKeyPem.value();
            if(deviceIdentityProfile.publishTopics.has_value()) existingEntity.value().publishTopics = deviceIdentityProfile.publishTopics.value();
            if(deviceIdentityProfile.subscribeTopics.has_value()) existingEntity.value().subscribeTopics = deviceIdentityProfile.subscribeTopics.value();
        }
        else {
            existingEntity = deviceIdentityProfile;
        }

        return Save(existingEntity.value());
    }


};

#endif // DEVICEIDENTITYPROFILEREPOSITORY_H