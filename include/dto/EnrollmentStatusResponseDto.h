#ifndef ENROLLMENTSTATUSRESPONSEDTO_H
#define ENROLLMENTSTATUSRESPONSEDTO_H

#include <StandardDefines.h>

/* @Serializable */
class EnrollmentStatusResponseDto {
    Public optional<Bool> enrolled;
    Public optional<StdString> tenantId;
    Public optional<StdString> thingName;
    Public optional<StdString> deviceType;
    Public optional<StdString> serialNumber;
    Public optional<StdString> enrolledAt;

    Public EnrollmentStatusResponseDto()
        : enrolled(),
          tenantId(),
          thingName(),
          deviceType(),
          serialNumber(),
          enrolledAt() {}
};

#endif // ENROLLMENTSTATUSRESPONSEDTO_H
