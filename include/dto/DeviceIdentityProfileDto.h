#ifndef DEVICEIDENTITYPROFILEDTO_H
#define DEVICEIDENTITYPROFILEDTO_H

#include <StandardDefines.h>
#include "../type/PublishTopics.h"
#include "../type/SubscribeTopics.h"

/* @Serializable */
class DeviceIdentityProfileDto {

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

#endif // DEVICEIDENTITYPROFILEDTO_H