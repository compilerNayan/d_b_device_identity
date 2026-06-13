#ifndef ENROLLMENTSTATUSRESPONSEDTO_H
#define ENROLLMENTSTATUSRESPONSEDTO_H

#include <StandardDefines.h>

/* @Serializable */
class EnrollmentStatusResponseDto {
    Public optional<Bool> enrolled;
    Public optional<Bool> failed;
    Public optional<StdString> failureReason;
    Public optional<StdString> tenantId;
    Public optional<StdString> thingName;
    Public optional<StdString> deviceType;
    Public optional<StdString> serialNumber;
    Public optional<StdString> enrolledAt;

    Public EnrollmentStatusResponseDto()
        : enrolled(),
          failed(),
          failureReason(),
          tenantId(),
          thingName(),
          deviceType(),
          serialNumber(),
          enrolledAt() {}
};

#endif // ENROLLMENTSTATUSRESPONSEDTO_H
