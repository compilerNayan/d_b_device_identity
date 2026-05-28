#ifndef DEVICEIDENTITYPROFILE_H
#define DEVICEIDENTITYPROFILE_H

#include <StandardDefines.h>
#include "../type/PublishTopics.h"
#include "../type/SubscribeTopics.h"

/* @Entity */
class DeviceIdentityProfile {

    /* @Id */
    /* @NotNull */
    Public optional<int> id;

    /* @NotNull */
    Public optional<StdString> mqttEndpoint;

    /* @NotNull */
    Public optional<StdString> caCertificatePem;

    /* @NotNull */
    Public optional<StdString> clientCertificatePem;

    /* @NotNull */
    Public optional<StdString> clientPrivateKeyPem;

    Public optional<PublishTopics> publishTopics;

    Public optional<SubscribeTopics> subscribeTopics;

};

#endif // DEVICEIDENTITYPROFILE_H