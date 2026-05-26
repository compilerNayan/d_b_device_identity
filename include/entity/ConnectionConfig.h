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

    Public optional<StdString> mqttEndpoint;

    Public optional<MqttCredentials> enrollmentCredentials;

    Public optional<MqttCredentials> connectionCredentials;

    Public optional<PublishTopics> publishTopics;

    Public optional<SubscribeTopics> subscribeTopics;

};

#endif // CONNECTIONCONFIG_H