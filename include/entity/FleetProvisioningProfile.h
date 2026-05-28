#ifndef FLEETPROVISIONINGPROFILE_H
#define FLEETPROVISIONINGPROFILE_H

#include <StandardDefines.h>

/* @Entity */
class FleetProvisioningProfile {

    /* @Id */
    /* @NotNull */
    Public optional<int> id;

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

#endif // FLEETPROVISIONINGPROFILE_H