#ifndef DEVICESERVICE_H
#define DEVICESERVICE_H

#include "IDeviceService.h"
#include "../repository/EnrollmentCredentialsRepository.h"
#include "../repository/ConnectionCredentialsRepository.h"
#include "../repository/PublishTopicsRepository.h"
#include "../repository/SubscribeTopicsRepository.h"

/* @Service */
class DeviceService : public IDeviceService {
    Public DeviceService() {
        Refresh();
    }
    Public Virtual ~DeviceService() = default;

    /* @Autowired */
    Private EnrollmentCredentialsRepositoryPtr enrollmentCredentialsRepository;

    /* @Autowired */
    Private ConnectionCredentialsRepositoryPtr connectionCredentialsRepository;

    /* @Autowired */
    Private PublishTopicsRepositoryPtr publishTopicsRepository;

    /* @Autowired */
    Private SubscribeTopicsRepositoryPtr subscribeTopicsRepository;

    Private StdString serialNumber;
    Private StdString deviceSecret;
    Private StdString firmwareVersion;

    Private MqttCredentialsDto enrollmentCredentials;
    Private Optional<MqttCredentialsDto> connectionCredentials;

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
        Val enrollmentCredentialsOpt = enrollmentCredentialsRepository->FindFirst();
        Val connectionCredentialsOpt = connectionCredentialsRepository->FindFirst();
        Val publishTopicsOpt = publishTopicsRepository->FindFirst();
        Val subscribeTopicsOpt = subscribeTopicsRepository->FindFirst();

        std::lock_guard<std::mutex> lock(mutex_);
        serialNumber = "1234";
        deviceSecret = "1234";
        firmwareVersion = "1.0.0";


        if (enrollmentCredentialsOpt.has_value()) {
            Val enrollmentCredentials = enrollmentCredentialsOpt.value();
            this->enrollmentCredentials = GetMqttCredentialsDto(enrollmentCredentials);
        } else {
            MqttCredentialsDto enrollmentCredentialsDto;
            enrollmentCredentialsDto.mqttEndpoint = "";
            enrollmentCredentialsDto.caCertificatePem = "";
            enrollmentCredentialsDto.clientCertificatePem = "";
            enrollmentCredentialsDto.clientPrivateKeyPem = "";
            this->enrollmentCredentials = enrollmentCredentialsDto;
        }

        if(connectionCredentialsOpt.has_value()) {
            Val connectionCredentials = connectionCredentialsOpt.value();
            this->connectionCredentials = GetMqttCredentialsDto(connectionCredentials);
        } else {
            this->connectionCredentials = std::nullopt;
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

    // Implemented interface methods with locking
    Public StdString GetSerialNumber() const override { std::lock_guard<std::mutex> lock(mutex_); return serialNumber; }
    Public StdString GetDeviceSecret() const override { std::lock_guard<std::mutex> lock(mutex_); return deviceSecret; }
    Public StdString GetFirmwareVersion() const override { std::lock_guard<std::mutex> lock(mutex_); return firmwareVersion; }

    Public MqttCredentialsDto GetEnrollmentCredentials() const override { std::lock_guard<std::mutex> lock(mutex_); return enrollmentCredentials; }
    Public Optional<MqttCredentialsDto> GetConnectionCredentials() const override { std::lock_guard<std::mutex> lock(mutex_); return connectionCredentials; }

    Public StdSet<StdString> GetSubscribeTopics() const override { std::lock_guard<std::mutex> lock(mutex_); return subscribeTopics; }
    Public StdString GetCommandTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return commandTopic; }
    Public StdString GetOtaUpdateTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return otaUpdateTopic; }
    Public StdString GetFeatureFlagTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return featureFlagTopic; }

    Public StdString GetStatusTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return statusTopic; }
    Public StdString GetTelemetryTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return telemetryTopic; }
    Public StdString GetLogsTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return logsTopic; }
    Public StdString GetEventsTopic() const override { std::lock_guard<std::mutex> lock(mutex_); return eventsTopic; }

    Public Void SetEnrollmentCredentials(const MqttCredentialsDto& enrollmentCredentials) override { 
        enrollmentCredentialsRepository->Update(GetEnrollmentCredentialsEntity(enrollmentCredentials));
        Refresh();
    }
    
    Public Void SetConnectionCredentials(const MqttCredentialsDto& connectionCredentials) override { 
        connectionCredentialsRepository->Update(GetConnectionCredentialsEntity(connectionCredentials));
        Refresh();
    }

    Private EnrollmentCredentials GetEnrollmentCredentialsEntity(const MqttCredentialsDto& enrollmentCredentials) {
        EnrollmentCredentials enrollmentCredentialsEntity;
        enrollmentCredentialsEntity.id = 1;
        enrollmentCredentialsEntity.mqttEndpoint = enrollmentCredentials.mqttEndpoint.value();
        enrollmentCredentialsEntity.caCertificatePem = enrollmentCredentials.caCertificatePem.value();
        enrollmentCredentialsEntity.clientCertificatePem = enrollmentCredentials.clientCertificatePem.value();
        enrollmentCredentialsEntity.clientPrivateKeyPem = enrollmentCredentials.clientPrivateKeyPem.value();
        return enrollmentCredentialsEntity;
    }

    Private ConnectionCredentials GetConnectionCredentialsEntity(const MqttCredentialsDto& connectionCredentials) {
        ConnectionCredentials connectionCredentialsEntity;
        connectionCredentialsEntity.id = 1;
        connectionCredentialsEntity.mqttEndpoint = connectionCredentials.mqttEndpoint.value();
        connectionCredentialsEntity.caCertificatePem = connectionCredentials.caCertificatePem.value();
        connectionCredentialsEntity.clientCertificatePem = connectionCredentials.clientCertificatePem.value();
        connectionCredentialsEntity.clientPrivateKeyPem = connectionCredentials.clientPrivateKeyPem.value();
        return connectionCredentialsEntity;
    }

    Private MqttCredentialsDto GetMqttCredentialsDto(const EnrollmentCredentials& enrollmentCredentials) {
        MqttCredentialsDto enrollmentCredentialsDto;
        enrollmentCredentialsDto.mqttEndpoint = enrollmentCredentials.mqttEndpoint;
        enrollmentCredentialsDto.caCertificatePem = enrollmentCredentials.caCertificatePem;
        enrollmentCredentialsDto.clientCertificatePem = enrollmentCredentials.clientCertificatePem;
        enrollmentCredentialsDto.clientPrivateKeyPem = enrollmentCredentials.clientPrivateKeyPem;
        return enrollmentCredentialsDto;
    }

    Private MqttCredentialsDto GetMqttCredentialsDto(const ConnectionCredentials& connectionCredentials) {
        MqttCredentialsDto connectionCredentialsDto;
        connectionCredentialsDto.mqttEndpoint = connectionCredentials.mqttEndpoint;
        connectionCredentialsDto.caCertificatePem = connectionCredentials.caCertificatePem;
        connectionCredentialsDto.clientCertificatePem = connectionCredentials.clientCertificatePem;
        connectionCredentialsDto.clientPrivateKeyPem = connectionCredentials.clientPrivateKeyPem;
        return connectionCredentialsDto;
    }
};
#endif // DEVICESERVICE_H