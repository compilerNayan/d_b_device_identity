#ifndef ENROLLMENTSERVICE_H
#define ENROLLMENTSERVICE_H

#include <ctime>
#include <mutex>
#include <StandardDefines.h>
#include "IEnrollmentService.h"
#include "../repository/DeviceEnrollmentRepository.h"
#include "../repository/DeviceIdentityProfileRepository.h"
#include "IConnectionDetailsProvider.h"

/* @Service */
class EnrollmentService final : public IEnrollmentService {
    Public EnrollmentService() = default;
    Public Virtual ~EnrollmentService() override = default;

    /* @Autowired */
    Private DeviceEnrollmentRepositoryPtr deviceEnrollmentRepository;

    /* @Autowired */
    Private DeviceIdentityProfileRepositoryPtr deviceIdentityProfileRepository;

    /* @Autowired */
    Private IConnectionDetailsProviderPtr connectionDetailsProvider;

    Private mutable std::mutex mutex_;
    Private Bool enrollmentFailed_ = false;
    Private StdString failureReason_;
    Private StdString failureCode_;

    Public EnrollmentNotifyResponseDto SavePostEnrollmentDetails(
            EnrollmentNotifyRequestDto request) override {
        EnrollmentNotifyResponseDto response;

        if (!request.tenantId.has_value() || request.tenantId.value().empty()) {
            response.enrolled = false;
            response.message = "tenantId is required";
            return response;
        }

        if (request.serialNumber.has_value() && !request.serialNumber.value().empty()) {
            const StdString localSerial = connectionDetailsProvider->GetSerialNumber();
            if (!localSerial.empty() &&
                localSerial != request.serialNumber.value()) {
                response.enrolled = false;
                response.message = "serialNumber does not match this device";
                return response;
            }
        }

        DeviceEnrollment enrollment;
        enrollment.id = 1;
        enrollment.tenantId = request.tenantId;
        enrollment.thingName = request.thingName;
        enrollment.deviceType = request.deviceType;
        enrollment.serialNumber = request.serialNumber.has_value() &&
                                          !request.serialNumber.value().empty()
                                  ? request.serialNumber
                                  : Optional<StdString>(
                                            connectionDetailsProvider->GetSerialNumber());
        enrollment.enrolledAt = request.enrolledAt.has_value() &&
                                        !request.enrolledAt.value().empty()
                                ? request.enrolledAt
                                : Optional<StdString>(FormatUtcTimestamp());

        deviceEnrollmentRepository->SaveEnrollment(enrollment);
        SyncDeviceIdentityProfile(enrollment);
        connectionDetailsProvider->Refresh();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            enrollmentFailed_ = false;
            failureReason_.clear();
            failureCode_.clear();
        }

        response.enrolled = true;
        response.tenantId = enrollment.tenantId;
        response.message = "Device enrollment saved";
        return response;
    }

    Public EnrollmentFailureResponseDto NotifyEnrollmentFailure(
            EnrollmentFailureRequestDto request) override {
        EnrollmentFailureResponseDto response;

        if (request.serialNumber.has_value() && !request.serialNumber.value().empty()) {
            const StdString localSerial = connectionDetailsProvider->GetSerialNumber();
            if (!localSerial.empty() &&
                localSerial != request.serialNumber.value()) {
                response.acknowledged = false;
                response.message = "serialNumber does not match this device";
                return response;
            }
        }

        StdString reason = request.reason.has_value() && !request.reason.value().empty()
                                   ? request.reason.value()
                                   : "Enrollment failed";
        if (request.code.has_value() && !request.code.value().empty()) {
            reason = request.code.value() + ": " + reason;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            enrollmentFailed_ = true;
            failureReason_ = reason;
            failureCode_ = request.code.has_value() ? request.code.value() : "";
        }

        response.acknowledged = true;
        response.message = "Enrollment failure recorded";
        return response;
    }

    Public Bool IsEnrolled() const override {
        Var enrollment = deviceEnrollmentRepository->FindFirst();
        return enrollment.has_value() &&
               enrollment.value().tenantId.has_value() &&
               !enrollment.value().tenantId.value().empty();
    }

    Public Bool HasEnrollmentFailed() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return enrollmentFailed_;
    }

    Public Void ClearEnrollmentFailure() override {
        std::lock_guard<std::mutex> lock(mutex_);
        enrollmentFailed_ = false;
        failureReason_.clear();
        failureCode_.clear();
    }

    Public Optional<StdString> GetTenantId() const override {
        Var enrollment = deviceEnrollmentRepository->FindFirst();
        if (!enrollment.has_value() ||
            !enrollment.value().tenantId.has_value() ||
            enrollment.value().tenantId.value().empty()) {
            return std::nullopt;
        }
        return enrollment.value().tenantId;
    }

    Public EnrollmentStatusResponseDto GetEnrollmentStatus() const override {
        EnrollmentStatusResponseDto status;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (enrollmentFailed_) {
                status.enrolled = false;
                status.failed = true;
                status.failureReason = failureReason_;
                return status;
            }
        }

        Var enrollment = deviceEnrollmentRepository->FindFirst();
        if (!enrollment.has_value()) {
            status.enrolled = false;
            status.failed = false;
            return status;
        }

        const DeviceEnrollment& saved = enrollment.value();
        status.enrolled = saved.tenantId.has_value() && !saved.tenantId.value().empty();
        status.failed = false;
        status.tenantId = saved.tenantId;
        status.thingName = saved.thingName;
        status.deviceType = saved.deviceType;
        status.serialNumber = saved.serialNumber;
        status.enrolledAt = saved.enrolledAt;
        return status;
    }

    Private Void SyncDeviceIdentityProfile(const DeviceEnrollment& enrollment) {
        DeviceIdentityProfile profileUpdate;
        profileUpdate.id = 1;
        profileUpdate.tenantId = enrollment.tenantId;
        if (enrollment.thingName.has_value() && !enrollment.thingName.value().empty()) {
            profileUpdate.thingName = enrollment.thingName;
        }
        if (enrollment.deviceType.has_value() && !enrollment.deviceType.value().empty()) {
            profileUpdate.deviceType = enrollment.deviceType;
        }
        deviceIdentityProfileRepository->UpdateAvailableFields(profileUpdate);
    }

    Private static StdString FormatUtcTimestamp() {
        time_t now = time(nullptr);
        struct tm utc;
        gmtime_r(&now, &utc);
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
        return StdString(buffer);
    }
};

#endif // ENROLLMENTSERVICE_H
