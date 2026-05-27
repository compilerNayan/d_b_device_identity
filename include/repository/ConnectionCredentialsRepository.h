#ifndef CONNECTIONCREDENTIALSREPOSITORY_H
#define CONNECTIONCREDENTIALSREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/ConnectionCredentials.h"

/* @Repository */
DefineStandardPointers(ConnectionCredentialsRepository)
class ConnectionCredentialsRepository : public CpaRepository<ConnectionCredentials, int> {
    Public Virtual ~ConnectionCredentialsRepository() = default;
};

#endif // CONNECTIONCREDENTIALSREPOSITORY_H