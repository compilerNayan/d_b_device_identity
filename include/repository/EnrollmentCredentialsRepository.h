#ifndef ENROLLMENTCREDENTIALSREPOSITORY_H
#define ENROLLMENTCREDENTIALSREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/EnrollmentCredentials.h"

/* @Repository */
DefineStandardPointers(EnrollmentCredentialsRepository)
class EnrollmentCredentialsRepository : public CpaRepository<EnrollmentCredentials, int> {
    Public Virtual ~EnrollmentCredentialsRepository() = default;
};

#endif // ENROLLMENTCREDENTIALSREPOSITORY_H