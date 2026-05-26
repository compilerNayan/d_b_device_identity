#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "IDeviceService.h"
#include "../repository/ConnectionConfigRepository.h"

/* @Service */
class DeviceService : public IDeviceService {
    Public DeviceService() {
        Refresh();
    }
    Public Virtual ~DeviceService() = default;

    /* @Autowired */
    Private ConnectionConfigRepositoryPtr connectionConfigRepository;

    Private StdString serialNumber;
    Private StdString deviceSecret;
    Private StdString firmwareVersion;
    Private StdString mqttEndpoint;

    Private MqttCredentials enrollmentCredentials;
    Private Optional<MqttCredentials> connectionCredentials;

    Private StdSet<StdString> subscribeTopics;
    Private StdString commandTopic;
    Private StdString otaUpdateTopic;
    Private StdString featureFlagTopic;

    Private StdSet<StdString> publishTopics;
    Private StdString statusTopic;
    Private StdString telemetryTopic;
    Private StdString logsTopic;
    Private StdString eventsTopic;

    Private mutable std::mutex mutex_;

    Public Void Refresh() override {
        Val connectionConfigOpt = connectionConfigRepository->FindFirst();

        std::lock_guard<std::mutex> lock(mutex_);
        serialNumber = "1234";
        deviceSecret = "1234";
        firmwareVersion = "1.0.0";

        if (connectionConfigOpt.has_value()) {
            Val connectionConfig = connectionConfigOpt.value();
            Val mqttEndpointOpt = connectionConfig->mqttEndpoint;
            Val enrollmentCredentialsOpt = connectionConfig->enrollmentCredentials;
            Val connectionCredentialsOpt = connectionConfig->connectionCredentials;
            Val publishTopicsOpt = connectionConfig->publishTopics;
            Val subscribeTopicsOpt = connectionConfig->subscribeTopics;
            this->mqttEndpoint = mqttEndpointOpt.has_value() ? mqttEndpointOpt.value() : "";

            MqttCredentials enrollmentCredentials = MqttCredentials();
            enrollmentCredentials.caCertificatePem = "";
            enrollmentCredentials.clientCertificatePem = "";
            enrollmentCredentials.clientPrivateKeyPem = "";

            if (enrollmentCredentialsOpt.has_value()) {
                Val enrollmentCredentials = enrollmentCredentialsOpt.value();
                enrollmentCredentials = enrollmentCredentials.value();
                this->enrollmentCredentials = enrollmentCredentials;
            }

            if (connectionCredentialsOpt.has_value()) {
                this->connectionCredentials = connectionCredentialsOpt.value();
            }

            if (publishTopicsOpt.has_value()) {
                Val publishTopics = publishTopicsOpt.value();
                this->statusTopic = publishTopics.statusTopic.has_value() ? publishTopics.statusTopic.value() : "";
                this->telemetryTopic = publishTopics.telemetryTopic.has_value() ? publishTopics.telemetryTopic.value() : "";
                this->logsTopic = publishTopics.logsTopic.has_value() ? publishTopics.logsTopic.value() : "";
                this->eventsTopic = publishTopics.eventsTopic.has_value() ? publishTopics.eventsTopic.value() : "";
            }

            if (subscribeTopicsOpt.has_value()) {
                Val subscribeTopics = subscribeTopicsOpt.value();
                this->commandTopic = subscribeTopics.commandTopic.has_value() ? subscribeTopics.commandTopic.value() : "";
                this->otaUpdateTopic = subscribeTopics.otaUpdateTopic.has_value() ? subscribeTopics.otaUpdateTopic.value() : "";
                this->featureFlagTopic = subscribeTopics.featureFlagTopic.has_value() ? subscribeTopics.featureFlagTopic.value() : "";
            }
        }
    }

    // Implemented interface methods with locking
    Public StdString GetSerialNumber() const override { std::lock_guard<std::mutex> lock(mutex_); return serialNumber; }
    Public StdString GetDeviceSecret() const override { std::lock_guard<std::mutex> lock(mutex_); return deviceSecret; }
    Public StdString GetFirmwareVersion() const override { std::lock_guard<std::mutex> lock(mutex_); return firmwareVersion; }
    Public StdString GetMqttEndpoint() const override { std::lock_guard<std::mutex> lock(mutex_); return mqttEndpoint; }

    Public MqttCredentials GetEnrollmentCredentials() const override { std::lock_guard<std::mutex> lock(mutex_); return enrollmentCredentials; }
    Public Optional<MqttCredentials> GetConnectionCredentials() const override { std::lock_guard<std::mutex> lock(mutex_); return connectionCredentials; }

    Public StdSet<StdString> GetSubscribeTopics() const override { std::lock_guard<std::mutex> lock(mutex_); return subscribeTopics; }
    Public StdString GetCommandTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return commandTopic; }
    Public StdString GetOtaUpdateTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return otaUpdateTopic; }
    Public StdString GetFeatureFlagTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return featureFlagTopic; }

    Public StdString GetStatusTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return statusTopic; }
    Public StdString GetTelemetryTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return telemetryTopic; }
    Public StdString GetLogsTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return logsTopic; }
    Public StdString GetEventsTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return eventsTopic; }


    Public Void SetEnrollmentCredentials(const MqttCredentials& enrollmentCredentials) override { 
        connectionConfigRepository->UpdateEnrollmentCredentials(enrollmentCredentials);
        Refresh();
    }
    
    Public Void SetConnectionCredentials(const MqttCredentials& connectionCredentials) override { 
        connectionConfigRepository->UpdateConnectionCredentials(connectionCredentials);
        Refresh();
    }
};
#endif // DEVICESERVICE_H