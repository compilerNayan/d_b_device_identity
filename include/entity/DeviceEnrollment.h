#ifndef DEVICEENROLLMENT_H
#define DEVICEENROLLMENT_H

#include <StandardDefines.h>

/* @Entity */
class DeviceEnrollment {
    /* @Id */
    /* @NotNull */
    Public optional<int> id;

    Public optional<StdString> tenantId;
    Public optional<StdString> thingName;
    Public optional<StdString> deviceType;
    Public optional<StdString> serialNumber;
    Public optional<StdString> enrolledAt;
};

#endif // DEVICEENROLLMENT_H
