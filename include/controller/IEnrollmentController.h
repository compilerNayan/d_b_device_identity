#ifndef IENROLLMENTCONTROLLER_DEVICE_IDENTITY_H
#define IENROLLMENTCONTROLLER_DEVICE_IDENTITY_H

#include <StandardDefines.h>
#include "../dto/EnrollmentNotifyRequestDto.h"
#include "../dto/EnrollmentNotifyResponseDto.h"
#include "../dto/EnrollmentStatusResponseDto.h"

DefineStandardPointers(IEnrollmentController)
class IEnrollmentController {
    Public Virtual ~IEnrollmentController() = default;

    /**
     * @brief Persist post-enrollment details (tenant assignment) on the device.
     */
    Public Virtual EnrollmentNotifyResponseDto NotifyEnrollment(
            EnrollmentNotifyRequestDto request) = 0;

    /**
     * @brief Returns whether the device has saved enrollment (tenantId present).
     */
    Public Virtual EnrollmentStatusResponseDto GetEnrollmentStatus() = 0;
};

#include "EnrollmentController.h"

#endif // IENROLLMENTCONTROLLER_DEVICE_IDENTITY_H
