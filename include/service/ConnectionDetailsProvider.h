#ifndef CONNECTIONDETAILSPROVIDER_H
#define CONNECTIONDETAILSPROVIDER_H

#include <StandardDefines.h>
#include "IConnectionDetailsProvider.h"
#include "../repository/FleetProvisioningProfileRepository.h"
#include "../repository/DeviceIdentityProfileRepository.h"

/* @Component */
class ConnectionDetailsProvider final : public IConnectionDetailsProvider {

    Public ConnectionDetailsProvider() {
        Refresh();
    }

    Public StdString GetSerialNumber() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return "1234";
    }

    Public StdString GetDeviceSecret() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return "1234";
    }

    Public StdString GetFirmwareVersion() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return "1.0.0";
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

            if (deviceIdentityProfileEntity.mqttEndpoint.has_value()) deviceIdentityMqttEndpoint = deviceIdentityProfileEntity.mqttEndpoint.value();

            if (deviceIdentityProfileEntity.tenantId.has_value()) tenantId = deviceIdentityProfileEntity.tenantId.value();
            if (deviceIdentityProfileEntity.deviceType.has_value()) deviceType = deviceIdentityProfileEntity.deviceType.value();
            if (deviceIdentityProfileEntity.thingName.has_value()) thingName = deviceIdentityProfileEntity.thingName.value();

            if (deviceIdentityProfileEntity.caCertificatePem.has_value()) deviceIdentityCaCertificatePem = deviceIdentityProfileEntity.caCertificatePem.value();
            if (deviceIdentityProfileEntity.clientCertificatePem.has_value()) deviceIdentityClientCertificatePem = deviceIdentityProfileEntity.clientCertificatePem.value();
            if (deviceIdentityProfileEntity.clientPrivateKeyPem.has_value()) deviceIdentityClientPrivateKeyPem = deviceIdentityProfileEntity.clientPrivateKeyPem.value();

            StdString deviceIdentityTopicsPrefix = tenantId + "/" + deviceType + "/" + thingName;

            if(deviceIdentityProfileEntity.publishTopics.has_value()) {
                if(deviceIdentityProfileEntity.publishTopics->statusTopic.has_value()) deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + "/status";
                if(deviceIdentityProfileEntity.publishTopics->telemetryTopic.has_value()) deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + "/telemetry";
                if(deviceIdentityProfileEntity.publishTopics->logsTopic.has_value()) deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + "/logs";
                if(deviceIdentityProfileEntity.publishTopics->eventsTopic.has_value()) deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + "/events";
            }

            if(deviceIdentityProfileEntity.subscribeTopics.has_value()) {
                if(deviceIdentityProfileEntity.subscribeTopics->commandTopic.has_value()) deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + "/command";
                if(deviceIdentityProfileEntity.subscribeTopics->otaUpdateTopic.has_value()) deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + "/ota/update";
                if(deviceIdentityProfileEntity.subscribeTopics->featureFlagTopic.has_value()) deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + "/feature/flag";
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
MIIDWTCCAkGgAwIBAgIUauxlGYRu7NTS62kAseuLDsE/Y/UwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDUyOTE3NDQw
NFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMRtXfQfMTsllAh6Alok
MossjUszWIa1jGX0F1dmiJR5DL4CwDXppwfH63av84Ktksd4SAcsn4qPwmi6Brhu
Z7mvLp7dKcAEg/4rcM/gmnnbGHpX2ialaKi+//rK9CbSIAQQj6gEc5xTQi65blar
N2geBCit9cKrjk0XcvZMpWPLjtPnkTNHng1/IqQiJiW/9iMTTbhjWZqQgUHnBbaL
NKdr0oAeTcWlaU9b0yqanQWhBUcsN6ciGQD3P3f/45Nm+0Tqk7JwfZ3tnYhsEB7t
wvPZkNiC8MHCRKVIJqU2eWSkc9W6Xx8iz8wpBn4JStWhptvqCMoh20bWSZB4Zh1Y
2VcCAwEAAaNgMF4wHwYDVR0jBBgwFoAUDM0GONaRXxKeVvJ8fttSwTuYExYwHQYD
VR0OBBYEFJveIoXVFdiOUUJERSZTNlCEBd5CMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBprQVO1kx/d4FxiQ0Uscm0r3Yv
2gphw6dbD/Q9vuf8GuCN9gGNFUllgjVpxaCG6/JuhMySgqGRpcFuJiuW8FBoZDRr
FDmOO+btQrcUBkpnPgbN3h+2+5EppGZLX/rnFeFwkWqdTrRDlyXB7lCjkc3jLqg/
W1t0m7RNivO3UZA/V4VA3m0SErQ4FBmLqiL1df7UHQRhuUSjfTaxr0z7YyEqLECf
jCNZRcw2ZolcvWjfj7nTRl7XdXkG2rCTGuAuqBq3w1dTZ8hJT3vYIlxSUH9b1KOi
vO99IL4QPXeO6v/Kr/jLpOIphQB4+ZPuhVBw2NSh+xxDL/wOi3/RTeFhlcQ+
-----END CERTIFICATE-----)";
    Private StdString fleetProvisioningClientPrivateKeyPem = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAxG1d9B8xOyWUCHoCWiQyiyyNSzNYhrWMZfQXV2aIlHkMvgLA
NemnB8frdq/zgq2Sx3hIByyfio/CaLoGuG5nua8unt0pwASD/itwz+CaedsYelfa
JqVoqL7/+sr0JtIgBBCPqARznFNCLrluVqs3aB4EKK31wquOTRdy9kylY8uO0+eR
M0eeDX8ipCImJb/2IxNNuGNZmpCBQecFtos0p2vSgB5NxaVpT1vTKpqdBaEFRyw3
pyIZAPc/d//jk2b7ROqTsnB9ne2diGwQHu3C89mQ2ILwwcJEpUgmpTZ5ZKRz1bpf
HyLPzCkGfglK1aGm2+oIyiHbRtZJkHhmHVjZVwIDAQABAoIBABanHjy3FDBkBwxr
NbY7knCmdK0FgOIYMdbYLZDwzDODOkr3ej89cPvevHL1m7moVtMWNghNxyBtr9fw
DDJc2S163H+JtfTlEGvRirMOjgIbmFkTpJ7sWIONc2vy1/FaLkMJved8i/CtUWae
D/GC0WL6gzRFy5/ltQWC/MkW0cQOU+bNr+e0M2YOcposIpuGrhFHlXkJRfTXNGV8
MsRhWptz+eSv66Wrxs+SVaomCTYtj4R2cksMdOacGKhhAJZ11EDTO36cxJLd+v/+
Kgc04NZOjIQ5mPfob7FGil4JApTim7XFONmwsXf4y9vjX3bryKpCln8wYj1y9yU6
oYJ0mgECgYEA6ktYnjNaQJKBxT+ig81oLluQ2kxjU2cgfTmbmwGTSTQ0NJKbXvkX
MQnf8F+RKfGkUpoaNQBTnKtdx6TjeO5PmrSmk3ZO+sp54dYF+seK9LvWz/XxHkzy
zZX+milY6HnWJHkJaH5PBzey7Sc01NyNjL7Atxtx9/oV/A6P7W2lZoECgYEA1p/x
jwgwMKBA4c8Icru+5g/NjVirCohZAlDCXVeMCi2O0vKqjQ9elI8mN5c5TC+8SO3E
FJ7a87qR+TKBHgvGd9hLK7sjxiy34w5r+e1FIfn6qgeNpjyr+zxUD51IpYO6k6JM
nYP7+8Vc1i7R6JV0T4+9YRFKbbQ4qVwu1MecQ9cCgYA5fGMx7cenMEtG6mYlkLCP
L0hLR5LXU+ZU012qmqhYvcZRX+dfVh6qmZGomMo1UGTRH55ugrYBKsrHwV1Ovqop
e0mjxNN8FVHyZ2DwNyWRBsWeIOoYVSj96F+f06VM5Rf1uvzj9pnNW407QvNjhvn0
HpmKU+GSxnm+bpRB3TVIAQKBgAgxlwFE6MISfCeixcnLjSAvXkOVLQJ77hn+h00U
+I2y6472ILi963h0dpsMtO1JrUCDg0beQbGoX8ZJvVAuVudgZI5I2LbKIkMS2+Bw
xIDlkEJJg4jjLBWtWfA2qv1WqBrWyWICFe6ovECZDP56JjSbYjmAdXz2smsUz9zj
uf2FAoGBAJlbo9tNE0u4wDIaHgTLUpRHgb8DmEEkwCZaCABfETOBZrqL0kxJH21e
SpaL6Yfe5hcAfG6XE+3IWkihueCz8ky6fi7jlOnV+A65YbcpmYoPv44ppuP25Qvy
K/N00MetiubAo1fiCoetzDF1FteCd9gt7Y3TqunkpOAKU4qvIZ9e
-----END RSA PRIVATE KEY-----)";
    
    Private StdString fleetProvisioningCreateKeysRequestTopic = "$aws/certificates/create/json";
    Private StdString fleetProvisioningCreateKeysAcceptedTopic = "$aws/certificates/create/json/accepted";
    Private StdString fleetProvisioningCreateKeysRejectedTopic = "$aws/certificates/create/json/rejected";

    Private StdString fleetProvisioningTemplateName = "SimbaTemplate";

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
    Private StdString deviceType = "switch";
    Private StdString thingName = "thing";

    Private StdString deviceIdentityTopicsPrefix = tenantId + "/" + deviceType + "/" + thingName;

    Private Bool isDeviceIdentityProfilePresent = false;

    Private StdString deviceIdentityMqttEndpoint;
    Private StdString deviceIdentityCaCertificatePem; 
    Private StdString deviceIdentityClientCertificatePem; 
    Private StdString deviceIdentityClientPrivateKeyPem; 

    Private StdString deviceIdentityPublishTopicsStatusTopic = deviceIdentityTopicsPrefix + "/status";
    Private StdString deviceIdentityPublishTopicsTelemetryTopic = deviceIdentityTopicsPrefix + "/telemetry";
    Private StdString deviceIdentityPublishTopicsLogsTopic = deviceIdentityTopicsPrefix + "/logs";
    Private StdString deviceIdentityPublishTopicsEventsTopic = deviceIdentityTopicsPrefix + "/events";

    Private StdString deviceIdentitySubscribeTopicsCommandTopic = deviceIdentityTopicsPrefix + "/command";
    Private StdString deviceIdentitySubscribeTopicsOtaUpdateTopic = deviceIdentityTopicsPrefix + "/ota/update";
    Private StdString deviceIdentitySubscribeTopicsFeatureFlagTopic = deviceIdentityTopicsPrefix + "/feature/flag";

    /* @Autowired */
    Private DeviceIdentityProfileRepositoryPtr deviceIdentityProfileRepository;
};


#endif