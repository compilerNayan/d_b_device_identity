#ifndef FLEETPROVISIONINGPROFILEREPOSITORY_H
#define FLEETPROVISIONINGPROFILEREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/FleetProvisioningProfile.h"

/* @Repository */
DefineStandardPointers(FleetProvisioningProfileRepository)
class FleetProvisioningProfileRepository : public CpaRepository<FleetProvisioningProfile, int> {
    Public Virtual ~FleetProvisioningProfileRepository() = default;

    Public FleetProvisioningProfile UpdateAvailableFields(FleetProvisioningProfile& fleetProvisioningProfile) {
        Var existingEntity = FindFirst();
        if(existingEntity.has_value()) {
            if(fleetProvisioningProfile.mqttEndpoint.has_value()) existingEntity.value().mqttEndpoint = fleetProvisioningProfile.mqttEndpoint.value();
            if(fleetProvisioningProfile.caCertificatePem.has_value()) existingEntity.value().caCertificatePem = fleetProvisioningProfile.caCertificatePem.value();
            if(fleetProvisioningProfile.clientCertificatePem.has_value()) existingEntity.value().clientCertificatePem = fleetProvisioningProfile.clientCertificatePem.value();
            if(fleetProvisioningProfile.clientPrivateKeyPem.has_value()) existingEntity.value().clientPrivateKeyPem = fleetProvisioningProfile.clientPrivateKeyPem.value();
            if(fleetProvisioningProfile.createKeysRequestTopic.has_value()) existingEntity.value().createKeysRequestTopic = fleetProvisioningProfile.createKeysRequestTopic.value();
            if(fleetProvisioningProfile.createKeysAcceptedTopic.has_value()) existingEntity.value().createKeysAcceptedTopic = fleetProvisioningProfile.createKeysAcceptedTopic.value();
            if(fleetProvisioningProfile.createKeysRejectedTopic.has_value()) existingEntity.value().createKeysRejectedTopic = fleetProvisioningProfile.createKeysRejectedTopic.value();
            if(fleetProvisioningProfile.provisionRequestTopic.has_value()) existingEntity.value().provisionRequestTopic = fleetProvisioningProfile.provisionRequestTopic.value();
            if(fleetProvisioningProfile.provisionAcceptedTopic.has_value()) existingEntity.value().provisionAcceptedTopic = fleetProvisioningProfile.provisionAcceptedTopic.value();
            if(fleetProvisioningProfile.provisionRejectedTopic.has_value()) existingEntity.value().provisionRejectedTopic = fleetProvisioningProfile.provisionRejectedTopic.value();
        }
        else {
            existingEntity = fleetProvisioningProfile;
        }
        return Save(existingEntity.value());
    }
};

#endif // FLEETPROVISIONINGPROFILEREPOSITORY_H