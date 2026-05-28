#ifndef IFLEETPROVISIONINGSERVICE_H
#define IFLEETPROVISIONINGSERVICE_H

#include <StandardDefines.h>
#include "EnrollmentStatus.h"

DefineStandardPointer(IFleetProvisioningService);
class IFleetProvisioningService {
    Public Virtual Bool IsEnrolled() = 0;
    Public Virtual Void EnrollDevice() = 0;
    Public Virtual EnrollmentStatus GetEnrollmentStatus() = 0;
    Public Virtual EnrollmentStatus WaitForEnrollment(Int timeoutMs) = 0;
    Public Virtual Void CloseConnection() = 0;
};

#endif // IFLEETPROVISIONINGSERVICE_H