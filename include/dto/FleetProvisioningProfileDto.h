#ifndef FLEETPROVISIONINGPROFILEDTO_H
#define FLEETPROVISIONINGPROFILEDTO_H

#include <StandardDefines.h>

/* @Serializable */
class FleetProvisioningProfileDto {

    Public optional<StdString> mqttEndpoint;

    Public optional<StdString> caCertificatePem;

    Public optional<StdString> clientCertificatePem;

    Public optional<StdString> clientPrivateKeyPem;

    Public optional<StdString> createKeysRequestTopic;

    Public optional<StdString> createKeysAcceptedTopic;

    Public optional<StdString> createKeysRejectedTopic;

    Public optional<StdString> provisionRequestTopic;

    Public optional<StdString> provisionAcceptedTopic;

    Public optional<StdString> provisionRejectedTopic;
};

#endif // FLEETPROVISIONINGPROFILEDTO_H