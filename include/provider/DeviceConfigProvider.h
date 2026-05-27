#ifndef DEVICECONFIGPROVIDER_H
#define DEVICECONFIGPROVIDER_H

#include "../service/IDeviceService.h"

/* @Component */
class DeviceConfigProvider final : public IDeviceConfigProvider {

    /* @Autowired */
    Private IDeviceServicePtr deviceService;

    Public DeviceConfigProvider(IDeviceServicePtr deviceService) {
        this->deviceService = deviceService;
    }

    Public StdString GetSerialNumber() const override {
        return deviceService->GetSerialNumber();
    }
    
    Public StdString GetDeviceSecret() const override {
        return deviceService->GetDeviceSecret();
    }
    
    Public StdString GetFirmwareVersion() const override {
        return deviceService->GetFirmwareVersion();
    }


    Public MqttCredentialsDto GetEnrollmentCredentials() const override {
        return deviceService->GetEnrollmentCredentials();
    }
    
    Public Optional<MqttCredentialsDto> GetConnectionCredentials() const override {
        return deviceService->GetConnectionCredentials();
    }
    
    Public StdSet<StdString> GetSubscribeTopics() const override {
        return deviceService->GetSubscribeTopics();
    }
    
    Public StdString GetCommandTopic() const override {
        return deviceService->GetCommandTopic();
    }
    
    Public StdString GetOtaUpdateTopic() const override {
        return deviceService->GetOtaUpdateTopic();
    }
    
    Public StdString GetFeatureFlagTopic() const override {
        return deviceService->GetFeatureFlagTopic();
    }
    
    Public StdString GetStatusTopic() const override {
        return deviceService->GetStatusTopic();
    }
    
    Public StdString GetTelemetryTopic() const override {
        return deviceService->GetTelemetryTopic();
    }
    
    Public StdString GetLogsTopic() const override {
        return deviceService->GetLogsTopic();
    }
    
    Public StdString GetEventsTopic() const override {
        return deviceService->GetEventsTopic();
    }
};

#endif // DEVICECONFIGPROVIDER_H