#ifndef ENROLLMENTFAILUREREQUESTDTO_H
#define ENROLLMENTFAILUREREQUESTDTO_H

#include <StandardDefines.h>

/* @Serializable */
class EnrollmentFailureRequestDto {
    Public optional<StdString> serialNumber;
    Public optional<StdString> reason;
    Public optional<StdString> code;

    Public EnrollmentFailureRequestDto()
        : serialNumber(), reason(), code() {}
};

#endif // ENROLLMENTFAILUREREQUESTDTO_H
