#ifndef CONNECTIONCONFIGREPOSITORY_H
#define CONNECTIONCONFIGREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/ConnectionConfig.h"

/* @Repository */
DefineStandardPointers(ConnectionConfigRepository)
class ConnectionConfigRepository : public CpaRepository<ConnectionConfig, int> {
    Public Virtual ~ConnectionConfigRepository() = default;

    Public Virtual Void UpdateEnrollmentCredentials(const MqttCredentials& enrollmentCredentials) {
        Var connectionConfigOpt = FindFirst();
        if (connectionConfigOpt.has_value()) {
            Var connectionConfig = connectionConfigOpt.value();
            connectionConfig.enrollmentCredentials = enrollmentCredentials;
            Update(connectionConfig);
        }
    }

    Public Virtual Void UpdateConnectionCredentials(const MqttCredentials& connectionCredentials) {
        Var connectionConfigOpt = FindFirst();
        if (connectionConfigOpt.has_value()) {
            Var connectionConfig = connectionConfigOpt.value();
            connectionConfig.connectionCredentials = connectionCredentials;
            Update(connectionConfig);
        }
    }
};

#endif // CONNECTIONCONFIGREPOSITORY_H