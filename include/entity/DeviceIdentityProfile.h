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

    Public optional<StdString> mqttEndpoint;

    Public optional<StdString> tenantId;
    Public optional<StdString> deviceType;
    Public optional<StdString> thingName;

    Public optional<StdString> caCertificatePem;
    Public optional<StdString> clientCertificatePem;
    Public optional<StdString> clientPrivateKeyPem;

    Public optional<PublishTopics> publishTopics;
    Public optional<SubscribeTopics> subscribeTopics;

};

#endif // DEVICEIDENTITYPROFILE_H