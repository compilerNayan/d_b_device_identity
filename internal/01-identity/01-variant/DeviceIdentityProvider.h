#ifndef DEVICEIDENTITYPROVIDER_INTERNAL_H
#define DEVICEIDENTITYPROVIDER_INTERNAL_H

#include <identity/IDeviceIdentityProvider.h>

#include "esp_mac.h"
#include <StandardDefines.h>

/* @Component */
class DeviceIdentityProvider : public IDeviceIdentityProvider {
    Public DeviceIdentityProvider() {
        GenerateSerialNumber();
        GenerateDeviceSecret();
        GenerateFirmwareVersion();
        GenerateDeviceType();
    }

    Public Virtual ~DeviceIdentityProvider() override = default;

    Public Virtual StdString GetSerialNumber() const override {
        return serialNumber;
    }

    Public Virtual StdString GetDeviceSecret() const override {
        return deviceSecret;
    }

    Public Virtual StdString GetFirmwareVersion() const override {
        return firmwareVersion;
    }

    Public Virtual StdString GetDeviceType() const override {
        return deviceType;
    }

    Private void uint64_to_base36(uint64_t value, char* out, size_t out_len) {
        const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        char buf[32];
        int i = 0;

        if (value == 0) {
            snprintf(out, out_len, "0");
            return;
        }

        while (value > 0 && i < (int)sizeof(buf) - 1) {
            buf[i++] = digits[value % 36];
            value /= 36;
        }

        // reverse into output
        int j = 0;
        while (i > 0 && j < (int)out_len - 1) {
            out[j++] = buf[--i];
        }
        out[j] = '\0';
    }

    Private Void GenerateSerialNumber() {
        // Get immutable factory MAC
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);

        uint64_t mac_val = ((uint64_t)mac[0] << 40) |
                           ((uint64_t)mac[1] << 32) |
                           ((uint64_t)mac[2] << 24) |
                           ((uint64_t)mac[3] << 16) |
                           ((uint64_t)mac[4] << 8)  |
                           ((uint64_t)mac[5]);

        char serialBuf[32];
        uint64_to_base36(mac_val, serialBuf, sizeof(serialBuf));

        serialNumber = StdString(serialBuf);

        // Hardcoded secret for now, replace later with actual secure value
    }

    Private Void GenerateDeviceSecret() {
        deviceSecret = "1234";
    }

    Private Void GenerateFirmwareVersion() {
        firmwareVersion = "1.0.0";
    }

    Private Void GenerateDeviceType() {
        deviceType = "water_meter";
    }

    Private StdString serialNumber;
    Private StdString deviceSecret;
    Private StdString firmwareVersion;
    Private StdString deviceType;
};

#endif // DEVICEIDENTITYPROVIDER_INTERNAL_H