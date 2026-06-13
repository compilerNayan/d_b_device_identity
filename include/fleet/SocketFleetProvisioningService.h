#ifndef SOCKETFLEETPROVISIONINGSERVICE_H
#define SOCKETFLEETPROVISIONINGSERVICE_H

#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <mutex>

#include <StandardDefines.h>
#include "logger/ILogger.h"
#include "communication/IServerProvider.h"
#include "communication/ICloudServer.h"
#include "IFleetProvisioningService.h"
#include "../service/IEnrollmentService.h"

/**
 * Socket-based fleet provisioning: asks the cloud to enroll the device via TCP,
 * then waits until EnrollmentService reports a saved tenantId.
 */
/* @Component */
class SocketFleetProvisioningService final : public IFleetProvisioningService {
    Public SocketFleetProvisioningService()
        : status(EnrollmentStatus::NotStarted) {}

    Public Virtual ~SocketFleetProvisioningService() override = default;

    /* @Autowired */
    Private IServerProviderPtr serverProvider;

    /* @Autowired */
    Private IEnrollmentServicePtr enrollmentService;

    /* @Autowired */
    Private ILoggerPtr logger;

    Private mutable std::mutex mutex_;
    Private EnrollmentStatus status;

    Public Bool IsEnrolled() override {
        return enrollmentService != nullptr && enrollmentService->IsEnrolled();
    }

    Public Void EnrollDevice() override {
        if (enrollmentService == nullptr || serverProvider == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Unknown;
            return;
        }

        if (enrollmentService->IsEnrolled()) {
            if (logger != nullptr) {
                logger->Info(Tag::Untagged, "Device already enrolled");
            }
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::AlreadyEnrolled;
            return;
        }

        ICloudServerPtr cloudServer = serverProvider->GetCloudServerPtr();
        if (cloudServer == nullptr) {
            if (logger != nullptr) {
                logger->Warning(Tag::Untagged, "Cloud server unavailable; cannot begin enrollment");
            }
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Unknown;
            return;
        }

        if (logger != nullptr) {
            logger->Info(Tag::Untagged, "Beginning socket enrollment request");
        }

        enrollmentService->ClearEnrollmentFailure();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::InProgress;
        }

        if (!cloudServer->BeginEnrollment("")) {
            if (logger != nullptr) {
                logger->Warning(Tag::Untagged, "Failed to queue enrollment_request");
            }
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Unknown;
        }
    }

    Public EnrollmentStatus GetEnrollmentStatus() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (enrollmentService != nullptr && enrollmentService->HasEnrollmentFailed()) {
            return EnrollmentStatus::Failed_Unknown;
        }
        if (enrollmentService != nullptr && enrollmentService->IsEnrolled()) {
            if (status == EnrollmentStatus::AlreadyEnrolled) {
                return EnrollmentStatus::AlreadyEnrolled;
            }
            return EnrollmentStatus::Success;
        }
        return status;
    }

    Public EnrollmentStatus WaitForEnrollment(Int timeoutMs) override {
        Const Int stepMs = 200;
        Int waitedMs = 0;

        while (waitedMs < timeoutMs) {
            if (enrollmentService != nullptr && enrollmentService->HasEnrollmentFailed()) {
                std::lock_guard<std::mutex> lock(mutex_);
                status = EnrollmentStatus::Failed_Unknown;
                return status;
            }

            if (enrollmentService != nullptr && enrollmentService->IsEnrolled()) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (status == EnrollmentStatus::AlreadyEnrolled) {
                    return EnrollmentStatus::AlreadyEnrolled;
                }
                status = EnrollmentStatus::Success;
                return status;
            }

            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (status != EnrollmentStatus::NotStarted &&
                    status != EnrollmentStatus::InProgress) {
                    return status;
                }
            }

            vTaskDelay(pdMS_TO_TICKS(stepMs));
            waitedMs += stepMs;
        }

        return GetEnrollmentStatus();
    }

    Public Void CloseConnection() override {
    }
};

#endif // ESP_PLATFORM

#endif // SOCKETFLEETPROVISIONINGSERVICE_H
