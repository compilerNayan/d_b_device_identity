#ifndef ENROLLMENTNOTIFYRESPONSEDTO_H
#define ENROLLMENTNOTIFYRESPONSEDTO_H

#include <StandardDefines.h>

/* @Serializable */
class EnrollmentNotifyResponseDto {
    Public optional<Bool> enrolled;
    Public optional<StdString> tenantId;
    Public optional<StdString> message;

    Public EnrollmentNotifyResponseDto()
        : enrolled(), tenantId(), message() {}
};

#endif // ENROLLMENTNOTIFYRESPONSEDTO_H
