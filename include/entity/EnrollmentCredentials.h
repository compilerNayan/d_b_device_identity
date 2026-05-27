#ifndef ENROLLMENTCREDENTIALS_H
#define ENROLLMENTCREDENTIALS_H

#include <StandardDefines.h>

/*--@Serializable--*/
class EnrollmentCredentials {

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

#endif // ENROLLMENTCREDENTIALS_H