#ifndef IENROLLMENTSERVICE_H
#define IENROLLMENTSERVICE_H

#include <StandardDefines.h>
#include "../dto/EnrollmentNotifyRequestDto.h"
#include "../dto/EnrollmentNotifyResponseDto.h"
#include "../dto/EnrollmentFailureRequestDto.h"
#include "../dto/EnrollmentFailureResponseDto.h"
#include "../dto/EnrollmentStatusResponseDto.h"

DefineStandardPointers(IEnrollmentService)
class IEnrollmentService {
    Public Virtual ~IEnrollmentService() = default;

    Public Virtual EnrollmentNotifyResponseDto SavePostEnrollmentDetails(
            EnrollmentNotifyRequestDto request) = 0;

    Public Virtual EnrollmentFailureResponseDto NotifyEnrollmentFailure(
            EnrollmentFailureRequestDto request) = 0;

    Public Virtual Bool IsEnrolled() const = 0;

    Public Virtual Bool HasEnrollmentFailed() const = 0;

    Public Virtual Void ClearEnrollmentFailure() = 0;

    Public Virtual Optional<StdString> GetTenantId() const = 0;

    Public Virtual EnrollmentStatusResponseDto GetEnrollmentStatus() const = 0;
};

#endif // IENROLLMENTSERVICE_H
