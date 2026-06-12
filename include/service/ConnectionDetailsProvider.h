#ifndef CONNECTIONDETAILSPROVIDER_H
#define CONNECTIONDETAILSPROVIDER_H

#include <StandardDefines.h>
#include "IConnectionDetailsProvider.h"
#include "../repository/FleetProvisioningProfileRepository.h"
#include "../repository/DeviceIdentityProfileRepository.h"
#include <identity/IDeviceIdentityProvider.h>

/* @Component */
class ConnectionDetailsProvider final : public IConnectionDetailsProvider {

    Public ConnectionDetailsProvider() {
        Refresh();
    }

    Public StdString GetSerialNumber() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return serialNumber;
    }

    Public StdString GetDeviceSecret() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceSecret;
    }

    Public StdString GetFirmwareVersion() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return firmwareVersion;
    }

    Public StdString GetDeviceType() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceType;
    }

    Public StdString GetTenantId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return tenantId;
    }

    Public StdString GetThingName() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return thingName;
    }

    Public StdString GetFleetProvisioningMqttEndpoint() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningMqttEndpoint;
    }

    Public StdString GetFleetProvisioningTemplateName() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningTemplateName;
    }

    Public StdString GetFleetProvisioningCaCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCaCertificatePem;
    }

    Public StdString GetFleetProvisioningClientCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningClientCertificatePem;
    }

    Public StdString GetFleetProvisioningClientPrivateKeyPem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningClientPrivateKeyPem;
    }

    Public StdString GetFleetProvisioningCreateKeysRequestTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCreateKeysRequestTopic;
    }

    Public StdString GetFleetProvisioningCreateKeysAcceptedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCreateKeysAcceptedTopic;
    }

    Public StdString GetFleetProvisioningCreateKeysRejectedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningCreateKeysRejectedTopic;
    }

    Public StdString GetFleetProvisioningProvisionRequestTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningProvisionRequestTopic;
    }

    Public StdString GetFleetProvisioningProvisionAcceptedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningProvisionAcceptedTopic;
    }

    Public StdString GetFleetProvisioningProvisionRejectedTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return fleetProvisioningProvisionRejectedTopic;
    }

    Public Bool IsDeviceIdentityProfilePresent() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return isDeviceIdentityProfilePresent;
    }

    Public StdString GetDeviceIdentityMqttEndpoint() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityMqttEndpoint;
    }

    Public StdString GetDeviceIdentityCaCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityCaCertificatePem;
    }

    Public StdString GetDeviceIdentityClientCertificatePem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityClientCertificatePem;
    }

    Public StdString GetDeviceIdentityClientPrivateKeyPem() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityClientPrivateKeyPem;
    }

    Public StdString GetDeviceIdentityPublishTopicsStatusTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsStatusTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsTelemetryTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsTelemetryTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsLogsTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsLogsTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsEventsTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsEventsTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsWater30mBucketTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsWater30mBucketTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsWater1sBucketTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsWater1sBucketTopic;
    }

    Public StdString GetDeviceIdentityPublishTopicsLifecycleEnrolledTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentityPublishTopicsLifecycleEnrolledTopic;
    }

    Public StdString GetDeviceIdentitySubscribeTopicsCommandTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentitySubscribeTopicsCommandTopic;
    }

    Public StdString GetDeviceIdentitySubscribeTopicsOtaUpdateTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentitySubscribeTopicsOtaUpdateTopic;
    }

    Public StdString GetDeviceIdentitySubscribeTopicsFeatureFlagTopic() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return deviceIdentitySubscribeTopicsFeatureFlagTopic;
    }

    Public Void Refresh() override {
        std::lock_guard<std::mutex> lock(mutex_);
        serialNumber = deviceIdentityProvider->GetSerialNumber();
        deviceSecret = deviceIdentityProvider->GetDeviceSecret();
        firmwareVersion = deviceIdentityProvider->GetFirmwareVersion();
        deviceType = deviceIdentityProvider->GetDeviceType();
        RefreshFleetProvisioningProfile();
        RefreshDeviceIdentityProfile();
    }

    Private Void RefreshFleetProvisioningProfile() {
        Val fleetProvisioningProfileEntityOpt = fleetProvisioningProfileRepository->FindFirst();
        if (fleetProvisioningProfileEntityOpt.has_value()) {
            Val fleetProvisioningProfileEntity = fleetProvisioningProfileEntityOpt.value();

            if (fleetProvisioningProfileEntity.mqttEndpoint.has_value()) fleetProvisioningMqttEndpoint = fleetProvisioningProfileEntity.mqttEndpoint.value();
            if (fleetProvisioningProfileEntity.templateName.has_value()) fleetProvisioningTemplateName = fleetProvisioningProfileEntity.templateName.value();
            if (fleetProvisioningProfileEntity.caCertificatePem.has_value()) fleetProvisioningCaCertificatePem = fleetProvisioningProfileEntity.caCertificatePem.value();
            if (fleetProvisioningProfileEntity.clientCertificatePem.has_value()) fleetProvisioningClientCertificatePem = fleetProvisioningProfileEntity.clientCertificatePem.value();
            if (fleetProvisioningProfileEntity.clientPrivateKeyPem.has_value()) fleetProvisioningClientPrivateKeyPem = fleetProvisioningProfileEntity.clientPrivateKeyPem.value();

            if (fleetProvisioningProfileEntity.templateName.has_value()) fleetProvisioningTemplateName = fleetProvisioningProfileEntity.templateName.value();

            if (fleetProvisioningProfileEntity.createKeysRequestTopic.has_value()) fleetProvisioningCreateKeysRequestTopic = fleetProvisioningProfileEntity.createKeysRequestTopic.value();
            if (fleetProvisioningProfileEntity.createKeysAcceptedTopic.has_value()) fleetProvisioningCreateKeysAcceptedTopic = fleetProvisioningProfileEntity.createKeysAcceptedTopic.value();
            if (fleetProvisioningProfileEntity.createKeysRejectedTopic.has_value()) fleetProvisioningCreateKeysRejectedTopic = fleetProvisioningProfileEntity.createKeysRejectedTopic.value();

            if (fleetProvisioningProfileEntity.provisionRequestTopic.has_value()) {
                fleetProvisioningProvisionRequestTopic = fleetProvisioningProfileEntity.provisionRequestTopic.value();
            } else {
                if (fleetProvisioningProfileEntity.provisionRequestTopicPrefix.has_value()) fleetProvisioningProvisionRequestTopicPrefix = fleetProvisioningProfileEntity.provisionRequestTopicPrefix.value();
                if (fleetProvisioningProfileEntity.provisionRequestTopicSuffix.has_value()) fleetProvisioningProvisionRequestTopicSuffix = fleetProvisioningProfileEntity.provisionRequestTopicSuffix.value();
                fleetProvisioningProvisionRequestTopic = fleetProvisioningProvisionRequestTopicPrefix + 
                                                        fleetProvisioningTemplateName + 
                                                        fleetProvisioningProvisionRequestTopicSuffix;
            } 

            if (fleetProvisioningProfileEntity.provisionAcceptedTopic.has_value()) {
                fleetProvisioningProvisionAcceptedTopic = fleetProvisioningProfileEntity.provisionAcceptedTopic.value();
            } else {
                if (fleetProvisioningProfileEntity.provisionAcceptedTopicPrefix.has_value()) fleetProvisioningProvisionAcceptedTopicPrefix = fleetProvisioningProfileEntity.provisionAcceptedTopicPrefix.value();
                if (fleetProvisioningProfileEntity.provisionAcceptedTopicSuffix.has_value()) fleetProvisioningProvisionAcceptedTopicSuffix = fleetProvisioningProfileEntity.provisionAcceptedTopicSuffix.value();
                fleetProvisioningProvisionAcceptedTopic = fleetProvisioningProvisionAcceptedTopicPrefix + 
                                                        fleetProvisioningTemplateName + 
                                                        fleetProvisioningProvisionAcceptedTopicSuffix;
            }
            
            if (fleetProvisioningProfileEntity.provisionRejectedTopic.has_value()) {
                fleetProvisioningProvisionRejectedTopic = fleetProvisioningProfileEntity.provisionRejectedTopic.value();
            } else {
                if (fleetProvisioningProfileEntity.provisionRejectedTopicPrefix.has_value()) fleetProvisioningProvisionRejectedTopicPrefix = fleetProvisioningProfileEntity.provisionRejectedTopicPrefix.value();
                if (fleetProvisioningProfileEntity.provisionRejectedTopicSuffix.has_value()) fleetProvisioningProvisionRejectedTopicSuffix = fleetProvisioningProfileEntity.provisionRejectedTopicSuffix.value();
                fleetProvisioningProvisionRejectedTopic = fleetProvisioningProvisionRejectedTopicPrefix + 
                                                        fleetProvisioningTemplateName + 
                                                        fleetProvisioningProvisionRejectedTopicSuffix;
            }
        } 
    }

    Private Void RefreshDeviceIdentityProfile() {
        Val deviceIdentityProfileEntityOpt = deviceIdentityProfileRepository->FindFirst();
        if (deviceIdentityProfileEntityOpt.has_value()) {
            isDeviceIdentityProfilePresent = true;
            Val deviceIdentityProfileEntity = deviceIdentityProfileEntityOpt.value();

            deviceIdentityMqttEndpoint = fleetProvisioningMqttEndpoint;
            deviceIdentityCaCertificatePem = fleetProvisioningCaCertificatePem; 

            if (deviceIdentityProfileEntity.mqttEndpoint.has_value() && !deviceIdentityProfileEntity.mqttEndpoint.value().empty()) {
                deviceIdentityMqttEndpoint = deviceIdentityProfileEntity.mqttEndpoint.value();
            }

            if (deviceIdentityProfileEntity.tenantId.has_value() && !deviceIdentityProfileEntity.tenantId.value().empty()) {
                tenantId = deviceIdentityProfileEntity.tenantId.value();
            }

            if (deviceIdentityProfileEntity.thingName.has_value() && !deviceIdentityProfileEntity.thingName.value().empty()) {
                thingName = deviceIdentityProfileEntity.thingName.value();
            }

            deviceIdentityTopicsPrefix = tenantId + "/" + deviceType + "/" + thingName;

            deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + "/status";
            deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + "/telemetry";
            deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + "/logs";
            deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + "/events";

            deviceIdentityPublishTopicsWater30mBucketTopic = deviceIdentityTopicsPrefix + "/water/30m";
            deviceIdentityPublishTopicsWater1sBucketTopic = deviceIdentityTopicsPrefix + "/water/1s";
            deviceIdentityPublishTopicsLifecycleEnrolledTopic = deviceIdentityTopicsPrefix + "/lifecycle/enrolled";
            
            deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + "/command";
            deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + "/ota/update";
            deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + "/feature/flag";

            if (deviceIdentityProfileEntity.thingName.has_value() && !deviceIdentityProfileEntity.thingName.value().empty()) {
                thingName = deviceIdentityProfileEntity.thingName.value();
            }

            if (deviceIdentityProfileEntity.caCertificatePem.has_value() && !deviceIdentityProfileEntity.caCertificatePem.value().empty()) {
                deviceIdentityCaCertificatePem = deviceIdentityProfileEntity.caCertificatePem.value();
            }
            if (deviceIdentityProfileEntity.clientCertificatePem.has_value() && !deviceIdentityProfileEntity.clientCertificatePem.value().empty()) {
                deviceIdentityClientCertificatePem = deviceIdentityProfileEntity.clientCertificatePem.value();
            }
            if (deviceIdentityProfileEntity.clientPrivateKeyPem.has_value() && !deviceIdentityProfileEntity.clientPrivateKeyPem.value().empty()) {
                deviceIdentityClientPrivateKeyPem = deviceIdentityProfileEntity.clientPrivateKeyPem.value();
            }

            if(deviceIdentityProfileEntity.publishTopics.has_value()) {
                if(deviceIdentityProfileEntity.publishTopics->statusTopic.has_value() && !deviceIdentityProfileEntity.publishTopics->statusTopic.value().empty()) {
                    deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.publishTopics->statusTopic.value();
                }
                if(deviceIdentityProfileEntity.publishTopics->telemetryTopic.has_value() && !deviceIdentityProfileEntity.publishTopics->telemetryTopic.value().empty()) {
                    deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.publishTopics->telemetryTopic.value();
                }
                if(deviceIdentityProfileEntity.publishTopics->logsTopic.has_value() && !deviceIdentityProfileEntity.publishTopics->logsTopic.value().empty()) {
                    deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.publishTopics->logsTopic.value();
                }
                if(deviceIdentityProfileEntity.publishTopics->eventsTopic.has_value() && !deviceIdentityProfileEntity.publishTopics->eventsTopic.value().empty()) {
                    deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.publishTopics->eventsTopic.value();
                }
                if(deviceIdentityProfileEntity.publishTopics->water30mBucketTopic.has_value() && !deviceIdentityProfileEntity.publishTopics->water30mBucketTopic.value().empty()) {
                    deviceIdentityPublishTopicsWater30mBucketTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.publishTopics->water30mBucketTopic.value();
                }
                if(deviceIdentityProfileEntity.publishTopics->water1sBucketTopic.has_value() && !deviceIdentityProfileEntity.publishTopics->water1sBucketTopic.value().empty()) {
                    deviceIdentityPublishTopicsWater1sBucketTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.publishTopics->water1sBucketTopic.value();
                }
                if(deviceIdentityProfileEntity.publishTopics->lifecycleEnrolledTopic.has_value() && !deviceIdentityProfileEntity.publishTopics->lifecycleEnrolledTopic.value().empty()) {
                    deviceIdentityPublishTopicsLifecycleEnrolledTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.publishTopics->lifecycleEnrolledTopic.value();
                }
            }

            if(deviceIdentityProfileEntity.subscribeTopics.has_value()) {
                if(deviceIdentityProfileEntity.subscribeTopics->commandTopic.has_value() && !deviceIdentityProfileEntity.subscribeTopics->commandTopic.value().empty()) {
                    deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.subscribeTopics->commandTopic.value();
                }
                if(deviceIdentityProfileEntity.subscribeTopics->otaUpdateTopic.has_value() && !deviceIdentityProfileEntity.subscribeTopics->otaUpdateTopic.value().empty()) {
                    deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.subscribeTopics->otaUpdateTopic.value();
                }
                if(deviceIdentityProfileEntity.subscribeTopics->featureFlagTopic.has_value() && !deviceIdentityProfileEntity.subscribeTopics->featureFlagTopic.value().empty()) {
                    deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + deviceIdentityProfileEntity.subscribeTopics->featureFlagTopic.value();
                }
            }
        } 
    }

    Private mutable std::mutex mutex_;

    Private StdString fleetProvisioningMqttEndpoint = "mqtts://a2hlcpmplecdfa-ats.iot.us-east-1.amazonaws.com";
    Private StdString fleetProvisioningCaCertificatePem = R"(-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----)";
    Private StdString fleetProvisioningClientCertificatePem = R"(-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUbe9HHYeW7k5DUp3y085n01KGZ98wDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDUzMDEwNTI0
MVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAN2q4rfW3EyPnv1Myk5o
RRu5PoYoYY4gCQMogdbTPqE0pioC4/hnlmz+PXmE2rZgfgGNTJ4wrpPcHrTOP7qN
CrqhedVKH+wM5NKIN1PsusYBg9R99BKj61eQyNeqjAaaqg2cQAO4P4Pf2lHQEpKY
bzrU9je2gZo6tqA4JDO1M5a32n3Zs/yJfnM3pz0W7Nl7m0hMkZ9o8WMPT7BQJCHL
fsywvc5/vWtQvKNA5pibIhpofydfMuswWYWixpGEYIlNFqWsX/HKWsOfp1gZ3dOj
PgDPzBNoV7l/pesFCSejUZrJOYQRDyNyjc8NT00JrYHerVFWMka8tyeYMCiZfERo
5LsCAwEAAaNgMF4wHwYDVR0jBBgwFoAUcE5mKDC71gZMJzxjByxFwPKFmdUwHQYD
VR0OBBYEFBe85GRyVeDflPn56VrNWr3M9aeBMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQDeqLjNYrSV+u30bPp8EWNSgsVU
vdPvaJ0CajklrSAhLS/T9khdETxAfn7nvQw/4N8Prlh60kaiwKyDGPrhl///wVxS
pjYquVI3eBjQ6ni2qnjBTH5SfiCrgEmgYNxq6Nn/1BkHpGWO913SqZIZGd+MO3oa
Obvh+tnFqFyvc9qkwqPgCDpFmj8bCBscoEhl4vt4EPZN9LQ6/f55aPvAKx0DXNRG
BOpohb2vdaaiYOyHHtPcu01RG+WkKO5+9/0mk220jAG5MtqBdaQpRz2dQCp1Q1ks
vEv8w06C05mSYIBwtjQMQu93GYeIOYsOuB50F2bY3estrhBXg4Naaojygsqf
-----END CERTIFICATE-----)";
    Private StdString fleetProvisioningClientPrivateKeyPem = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEogIBAAKCAQEA3arit9bcTI+e/UzKTmhFG7k+hihhjiAJAyiB1tM+oTSmKgLj
+GeWbP49eYTatmB+AY1MnjCuk9wetM4/uo0KuqF51Uof7Azk0og3U+y6xgGD1H30
EqPrV5DI16qMBpqqDZxAA7g/g9/aUdASkphvOtT2N7aBmjq2oDgkM7Uzlrfafdmz
/Il+czenPRbs2XubSEyRn2jxYw9PsFAkIct+zLC9zn+9a1C8o0DmmJsiGmh/J18y
6zBZhaLGkYRgiU0Wpaxf8cpaw5+nWBnd06M+AM/ME2hXuX+l6wUJJ6NRmsk5hBEP
I3KNzw1PTQmtgd6tUVYyRry3J5gwKJl8RGjkuwIDAQABAoIBAB1sQFm8mFNFQQpI
NhZAOuQaK5VtKL3PvMKBjvJv6cFGFsQ+y/m97jabbJeDrfBFUJRuJ/xbY+DWd/Dx
632cmQ76vgw0oZYYhAr577YhFw2PR+tdqJcM0QE3g6E0zw9VWjsiQVD9FNkxTm5L
mxuARktd1yy/+eX54yHTMeL3K1jW4VU/3PxprEgaFgEJ9ZBMeidMl0kB3WNTi4ys
MyzoZc8MAUZXVYlkYe6fsS7Cn9t2BeJzxIYGtzTs4xjs0v/bb6lxmi96dP1oWwpZ
vI9b+ZEOnAp70s++W+SPHby0gayipMxReIlhUVhWXoymoDk98VnXhdOyiDw0+4oT
86501yECgYEA84YptgAMzYXTMqLF4lyYbO9tFSniDijBtxIT5TLEbj8/kWn4zhvr
Yx4uWaPEM6UQ7hex93JcwElxRHFPSIGyG8PnA6LlhzqtrPwoCDdM+DNVu4tq42ec
9OlcUp/SUmW8oxaZbW0VcD1UU1OS0SwLDiSjIhKUUYjyOKTIRgvHbGsCgYEA6QYS
gywc2db8gqL4PKf4RKGZSBZ/UBQqX6rPCl71F5KoDGS+nhDNsbekWI9gDQhA5Xpl
MQ9vebK6tD+UpqaB5DmyLxp2ZWq2OX4Hlbfd0sPZNF9h6khgAmiZqo7uU3ycPuDg
7oaL4sBgHO8Zqe2Gy6XNWlP9Mpkb+7ivpAl4fPECgYB/0DHCOWJ+2DdSA0azGQBT
ZJKvIe1omxGZIV7Z0/xvFLkrfCA/JT41JpkTKTYIGSG6pSseAaMWtTVCw+nl11SA
6CAus2eewzh2a14jecrnFiJwLatrMW2ayYRQRVvhLU6Flo3udetjnnzMwzdym5gt
0yLf9jpsVOE0w5/ty67egwKBgFcNSK8uNJ0A3pZjEX9/dJUXFa9DkE43KllQ80W5
kbA7voHaxQdB2cYRh9j5vvU/ZxcTcWgxjwCUz4D027CiNZYwI6vLI/3hLrAtr+Gz
ra/GMIeLNoYSgaOEthtsiAYyYCBxXDZflzSfj4hfnmPH83pyt1OOWuGjJzwTk7Ih
Q5zRAoGAdfizZDOSiK28g/6MjgNiIJlUANd61WjnO+xRly0e53vzRB02Ynrq/Vwu
kf2ro8Wle2TBVBCR0GpyKLFRl/V/axjXhkSCk0q4+RVEqLunw77PUspR7+cGLxVb
ok5rte626z1PeQc30Rtf45RMIiKla3iGOTsIX02gipx9a7vSyQg=
-----END RSA PRIVATE KEY-----)";
    
    Private StdString fleetProvisioningCreateKeysRequestTopic = "$aws/certificates/create/json";
    Private StdString fleetProvisioningCreateKeysAcceptedTopic = "$aws/certificates/create/json/accepted";
    Private StdString fleetProvisioningCreateKeysRejectedTopic = "$aws/certificates/create/json/rejected";

    Private StdString fleetProvisioningTemplateName = "TesterTempl";

    Private StdString fleetProvisioningProvisionRequestTopicPrefix = "$aws/provisioning-templates/";
    Private StdString fleetProvisioningProvisionRequestTopicSuffix = "/provision/json";
    Private StdString fleetProvisioningProvisionRequestTopic = fleetProvisioningProvisionRequestTopicPrefix + fleetProvisioningTemplateName + fleetProvisioningProvisionRequestTopicSuffix;

    Private StdString fleetProvisioningProvisionAcceptedTopicPrefix = "$aws/provisioning-templates/";
    Private StdString fleetProvisioningProvisionAcceptedTopicSuffix = "/provision/json/accepted";
    Private StdString fleetProvisioningProvisionAcceptedTopic = fleetProvisioningProvisionAcceptedTopicPrefix + fleetProvisioningTemplateName + fleetProvisioningProvisionAcceptedTopicSuffix;

    Private StdString fleetProvisioningProvisionRejectedTopicPrefix = "$aws/provisioning-templates/";
    Private StdString fleetProvisioningProvisionRejectedTopicSuffix = "/provision/json/rejected";
    Private StdString fleetProvisioningProvisionRejectedTopic = fleetProvisioningProvisionRejectedTopicPrefix + fleetProvisioningTemplateName + fleetProvisioningProvisionRejectedTopicSuffix;

    /* @Autowired */
    Private FleetProvisioningProfileRepositoryPtr fleetProvisioningProfileRepository;

    Private StdString tenantId = "123";
    Private StdString thingName = "thing";

    Private StdString deviceIdentityTopicsPrefix;

    Private Bool isDeviceIdentityProfilePresent = false;

    Private StdString deviceIdentityMqttEndpoint = fleetProvisioningMqttEndpoint;
    Private StdString deviceIdentityCaCertificatePem = fleetProvisioningCaCertificatePem; 
    Private StdString deviceIdentityClientCertificatePem; 
    Private StdString deviceIdentityClientPrivateKeyPem; 

    Private StdString deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + "/status";
    Private StdString deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + "/telemetry";
    Private StdString deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + "/logs";
    Private StdString deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + "/events";

    Private StdString deviceIdentityPublishTopicsWater30mBucketTopic = deviceIdentityTopicsPrefix + "/water/30m";
    Private StdString deviceIdentityPublishTopicsWater1sBucketTopic = deviceIdentityTopicsPrefix + "/water/1s";
    Private StdString deviceIdentityPublishTopicsLifecycleEnrolledTopic = deviceIdentityTopicsPrefix + "/lifecycle/enrolled";

    Private StdString deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + "/command";
    Private StdString deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + "/ota/update";
    Private StdString deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + "/feature/flag";

    /* @Autowired */
    Private DeviceIdentityProfileRepositoryPtr deviceIdentityProfileRepository;

    /* @Autowired */
    Private IDeviceIdentityProviderPtr deviceIdentityProvider;

    Private StdString serialNumber;
    Private StdString deviceSecret;
    Private StdString deviceType;
    Private StdString firmwareVersion;
};


#endif