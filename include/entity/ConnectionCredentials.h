#ifndef CONNECTIONCREDENTIALS_H
#define CONNECTIONCREDENTIALS_H

#include <StandardDefines.h>

/* @Entity */
class ConnectionCredentials {

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

};

#endif // CONNECTIONCREDENTIALS_H