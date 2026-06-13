#ifndef ENROLLMENTCONTROLLER_DEVICE_IDENTITY_H
#define ENROLLMENTCONTROLLER_DEVICE_IDENTITY_H

#include <StandardDefines.h>
#include "IEnrollmentController.h"
#include "../service/IEnrollmentService.h"
#include "logger/ILogger.h"

/**
 * Notifies the device that cloud enrollment completed and persists tenant details.
 *
 * POST /device-enrollment/notify
 * Body: EnrollmentNotifyRequestDto
 *
 * POST /device-enrollment/failure
 * Body: EnrollmentFailureRequestDto
 *
 * GET /device-enrollment/status
 */
/* @RestController */
/* @RequestMapping("/device-enrollment") */
class EnrollmentController final : public IEnrollmentController {
    /* @Autowired */
    Private IEnrollmentServicePtr enrollmentService;

    /* @Autowired */
    Private ILoggerPtr logger;

    Public EnrollmentController() = default;
    Public Virtual ~EnrollmentController() override = default;

    /* @PostMapping("/notify") */
    Public Virtual EnrollmentNotifyResponseDto NotifyEnrollment(
            /* @RequestBody */ EnrollmentNotifyRequestDto request) override {
        if (logger != nullptr) {
            const StdString tenantId =
                    request.tenantId.has_value() ? request.tenantId.value() : "";
            logger->Info(
                    Tag::Untagged,
                    "[EnrollmentController] notify tenantId=" + tenantId);
        }
        return enrollmentService->SavePostEnrollmentDetails(request);
    }

    /* @PostMapping("/failure") */
    Public Virtual EnrollmentFailureResponseDto NotifyEnrollmentFailure(
            /* @RequestBody */ EnrollmentFailureRequestDto request) override {
        if (logger != nullptr) {
            const StdString reason =
                    request.reason.has_value() ? request.reason.value() : "";
            logger->Warning(
                    Tag::Untagged,
                    "[EnrollmentController] failure reason=" + reason);
        }
        return enrollmentService->NotifyEnrollmentFailure(request);
    }

    /* @GetMapping("/status") */
    Public Virtual EnrollmentStatusResponseDto GetEnrollmentStatus() override {
        return enrollmentService->GetEnrollmentStatus();
    }
};

#endif // ENROLLMENTCONTROLLER_DEVICE_IDENTITY_H
