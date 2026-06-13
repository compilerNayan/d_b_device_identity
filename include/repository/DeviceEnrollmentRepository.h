#ifndef DEVICEENROLLMENTREPOSITORY_H
#define DEVICEENROLLMENTREPOSITORY_H

#include <StandardDefines.h>
#include "CpaRepository.h"
#include "../entity/DeviceEnrollment.h"

/* @Repository */
DefineStandardPointers(DeviceEnrollmentRepository)
class DeviceEnrollmentRepository : public CpaRepository<DeviceEnrollment, int> {
    Public Virtual ~DeviceEnrollmentRepository() = default;

    Public DeviceEnrollment SaveEnrollment(DeviceEnrollment& enrollment) {
        Var existingEntity = FindFirst();
        if (existingEntity.has_value()) {
            if (enrollment.tenantId.has_value()) {
                existingEntity.value().tenantId = enrollment.tenantId.value();
            }
            if (enrollment.thingName.has_value()) {
                existingEntity.value().thingName = enrollment.thingName.value();
            }
            if (enrollment.deviceType.has_value()) {
                existingEntity.value().deviceType = enrollment.deviceType.value();
            }
            if (enrollment.serialNumber.has_value()) {
                existingEntity.value().serialNumber = enrollment.serialNumber.value();
            }
            if (enrollment.enrolledAt.has_value()) {
                existingEntity.value().enrolledAt = enrollment.enrolledAt.value();
            }
        } else {
            enrollment.id = 1;
            existingEntity = enrollment;
        }
        return Save(existingEntity.value());
    }
};

#endif // DEVICEENROLLMENTREPOSITORY_H
