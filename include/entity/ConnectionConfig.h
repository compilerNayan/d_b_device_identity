#ifndef CONNECTIONCONFIG_H
#define CONNECTIONCONFIG_H

#include <StandardDefines.h>
#include "MqttCredentials.h"
#include "PublishTopics.h"
#include "SubscribeTopics.h"

/* @Entity */
class ConnectionConfig {

    /* @Id */
    Public optional<int> id;

    /* @NotNull */
    optional<StdString> mqttEndpoint;

    /* @NotNull */
    optional<MqttCredentials> enrollmentCredentials;

    /* @NotNull */
    optional<MqttCredentials> connectionCredentials;

    /* @NotNull */
    optional<PublishTopics> publishTopics;

    /* @NotNull */
    optional<SubscribeTopics> subscribeTopics;

};

#endif // CONNECTIONCONFIG_H