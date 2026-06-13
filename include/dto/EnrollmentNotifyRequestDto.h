#ifndef ENROLLMENTNOTIFYREQUESTDTO_H
#define ENROLLMENTNOTIFYREQUESTDTO_H

#include <StandardDefines.h>

/* @Serializable */
class EnrollmentNotifyRequestDto {
    Public optional<StdString> tenantId;
    Public optional<StdString> thingName;
    Public optional<StdString> deviceType;
    Public optional<StdString> serialNumber;
    Public optional<StdString> enrolledAt;

    Public EnrollmentNotifyRequestDto()
        : tenantId(), thingName(), deviceType(), serialNumber(), enrolledAt() {}
};

#endif // ENROLLMENTNOTIFYREQUESTDTO_H
