#ifndef MQTTCREDENTIALSDTO_H
#define MQTTCREDENTIALSDTO_H

#include <StandardDefines.h>

/* @Dto */
class MqttCredentialsDto {

    /* @NotNull */
    Public optional<StdString> mqttEndpoint;

    /* @NotNull */
    Public optional<StdString> caCertificatePem;

    /* @NotNull */
    Public optional<StdString> clientCertificatePem;

    /* @NotNull */
    Public optional<StdString> clientPrivateKeyPem;

};

#endif // MQTTCREDENTIALSDTO_H