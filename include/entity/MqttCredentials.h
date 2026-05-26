#ifndef MQTTCREDENTIALS_H
#define MQTTCREDENTIALS_H

#include <StandardDefines.h>

/* @Entity */
class MqttCredentials {

    /* @NotNull */
    Public optional<StdString> caCertificatePem;

    /* @NotNull */
    Public optional<StdString> clientCertificatePem;

    /* @NotNull */
    Public optional<StdString> clientPrivateKeyPem;
};

#endif // MQTTCREDENTIALS_H
