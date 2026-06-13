#ifndef ENROLLMENTFAILURERESPONSEDTO_H
#define ENROLLMENTFAILURERESPONSEDTO_H

#include <StandardDefines.h>

/* @Serializable */
class EnrollmentFailureResponseDto {
    Public optional<Bool> acknowledged;
    Public optional<StdString> message;

    Public EnrollmentFailureResponseDto()
        : acknowledged(), message() {}
};

#endif // ENROLLMENTFAILURERESPONSEDTO_H
