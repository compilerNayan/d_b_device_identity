#ifndef FLEETPROVISIONINGSERVICE_H
#define FLEETPROVISIONINGSERVICE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <mutex>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_tls.h"
#include "sdkconfig.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "StandardDefines.h"
#include "server/IDeviceService.h"
#include "logger/ILogger.h"

#include "IFleetProvisioningService.h"

/* @Component */
class FleetProvisioningService : public IFleetProvisioningService {
    
    Public FleetProvisioningService()
        : mqttClient(nullptr),
          mqttStarted(false),
          enrollmentStarted(false),
          pendingSubscriptions(0),
          status(EnrollmentStatus::NotStarted) {
            fpProfile = deviceService->GetFleetProvisioningProfile();
    }

    Public Bool IsEnrolled() override {
        return deviceService->GetDeviceIdentityProfile().has_value();
    }

    Public Void EnrollDevice() override {
        if(IsEnrolled()) {
            logger->Info(Tag::Untagged, "Device already enrolled");
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::AlreadyEnrolled;
            return;
        }
        
        logger->Info(Tag::Untagged,
                     "Starting enrollment for serial=" +
                     deviceService->GetSerialNumber());

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::InProgress;
        }

        fpProfile = deviceService->GetFleetProvisioningProfile();
        printf("[Nayan] EnrollDevice serial=%s endpoint_raw=%s\n",
            deviceService->GetSerialNumber().c_str(),
            fpProfile.mqttEndpoint.c_str());
        fflush(stdout);
        CloseConnection();
        StartMqttClient();
    }

    Public EnrollmentStatus GetEnrollmentStatus() override {
        std::lock_guard<std::mutex> lock(mutex_);
        return status;
    }

    Public EnrollmentStatus WaitForEnrollment(Int timeoutMs) override {
        Const Int step = 200;
        Int waited = 0;
        while (waited < timeoutMs) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (status != EnrollmentStatus::NotStarted &&
                    status != EnrollmentStatus::InProgress) {
                    return status;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(step));
            waited += step;
        }
        return GetEnrollmentStatus();
    }

    Public Void CloseConnection() override {
        printf("[Nayan] CloseConnection mqttClient=%p mqttStarted=%d\n",
            (void*)mqttClient, mqttStarted ? 1 : 0);
        fflush(stdout);
        std::lock_guard<std::mutex> lock(mutex_);
        if (mqttClient) {
            logger->Info(Tag::Untagged, "Closing MQTT connection...");
            esp_mqtt_client_stop(mqttClient);
            esp_mqtt_client_destroy(mqttClient);
            mqttClient = nullptr;
            mqttStarted = false;
        }
    }

    /* @Autowired */
    Private IDeviceServicePtr deviceService;

    /* @Autowired */
    Private ILoggerPtr logger;

    FleetProvisioningProfileData fpProfile;

    Private StdString enrollmentBrokerUri;
    Private StdString enrollmentBrokerHost;
    Private StdString enrollmentClientId;

    static constexpr Int kEnrollmentNetworkTimeoutMs = 30000;

    Private esp_mqtt_client_handle_t mqttClient;
    Private Bool mqttStarted;
    Private Bool enrollmentStarted;
    Private Int pendingSubscriptions;
    Private std::mutex mutex_;
    Private EnrollmentStatus status;

    // Buffers for received credentials
    Private StdString devicePrivateKeyPem;
    Private StdString awsDeviceCertPem;
    Private StdString awsCaCertificatePem;
    Private StdString ownershipToken;
    Private StdString thingName;
    Private StdString pendingTenantId;

    // Parsing buffers (heap-backed via singleton; not on mqtt_task stack)
    Private Char ownershipBuf[768];
    Private Char certIdBuf[128];
    Private Char keyEscBuf[4096];
    Private Char certEscBuf[4096];
    Private Char keyPemBuf[4096];
    Private Char certPemBuf[4096];

    Private Static Void EnrollmentCompleteTask(Void* arg) {
        Var self = static_cast<FleetProvisioningService*>(arg);
        const StdString tenantId = self->pendingTenantId;
        // Tear down enrollment MQTT/TLS before SPIFFS + JSON serialize (needs large heap).
        self->CloseConnection();
        vTaskDelay(pdMS_TO_TICKS(500));
        NayanLogWifiAndHeap("enroll_save_after_close");
        self->SaveReceivedCredentials(tenantId);
        vTaskDelete(nullptr);
    }

    Private bool json_extract_string(const char *json, const char *key, char *out, size_t out_len) {
        char pattern[64];
        snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
        const char *start = strstr(json, pattern);
        if (!start) {
            return false;
        }
        start += strlen(pattern);
        const char *end = strchr(start, '"');
        if (!end || static_cast<size_t>(end - start) >= out_len) {
            return false;
        }
        memcpy(out, start, static_cast<size_t>(end - start));
        out[end - start] = '\0';
        return true;
    }
    
    Private void unescape_json_to_pem(const char *escaped, char *out, size_t out_len) {
        size_t j = 0;
        for (size_t i = 0; escaped[i] != '\0' && j + 1 < out_len; ++i) {
            if (escaped[i] == '\\' && escaped[i + 1] == 'n') {
                out[j++] = '\n';
                ++i;
            } else if (escaped[i] == '\\' && escaped[i + 1] == '\\') {
                out[j++] = '\\';
                ++i;
            } else {
                out[j++] = escaped[i];
            }
        }
        out[j] = '\0';
    }

    /** Ensures mqtts URI includes explicit port (ESP-TLS requires it for AWS IoT). */
    Private Static StdString NormalizeMqttUri(const StdString& endpoint) {
        StdString uri = endpoint;
        if (uri.find("://") == StdString::npos) {
            return "mqtts://" + uri + ":8883";
        }
        const size_t hostStart = uri.find("://") + 3;
        const size_t slash = uri.find('/', hostStart);
        const size_t colon = uri.find(':', hostStart);
        const Bool hasPort = (colon != StdString::npos && (slash == StdString::npos || colon < slash));
        if (!hasPort) {
            if (slash != StdString::npos) {
                uri.insert(slash, ":8883");
            } else {
                uri += ":8883";
            }
        }
        return uri;
    }

    Private Static CChar* MqttEventIdName(Int32 eventId) {
        switch (eventId) {
            case MQTT_EVENT_CONNECTED: return "CONNECTED";
            case MQTT_EVENT_DISCONNECTED: return "DISCONNECTED";
            case MQTT_EVENT_SUBSCRIBED: return "SUBSCRIBED";
            case MQTT_EVENT_UNSUBSCRIBED: return "UNSUBSCRIBED";
            case MQTT_EVENT_PUBLISHED: return "PUBLISHED";
            case MQTT_EVENT_DATA: return "DATA";
            case MQTT_EVENT_ERROR: return "ERROR";
            case MQTT_EVENT_BEFORE_CONNECT: return "BEFORE_CONNECT";
            case MQTT_EVENT_DELETED: return "DELETED";
            default: return "OTHER";
        }
    }

    Private Static Void NayanLogWifiAndHeap(CChar* where) {
        printf("[Nayan] %s heap_free=%lu heap_min=%lu\n", where,
            (unsigned long)esp_get_free_heap_size(),
            (unsigned long)esp_get_minimum_free_heap_size());
        esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (!netif) {
            printf("[Nayan] %s WIFI_STA_DEF netif=null\n", where);
        } else {
            esp_netif_ip_info_t ip{};
            if (esp_netif_get_ip_info(netif, &ip) == ESP_OK) {
                printf("[Nayan] %s STA ip=" IPSTR " gw=" IPSTR "\n", where,
                    IP2STR(&ip.ip), IP2STR(&ip.gw));
            } else {
                printf("[Nayan] %s esp_netif_get_ip_info failed\n", where);
            }
        }
        wifi_ap_record_t ap{};
        if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
            printf("[Nayan] %s WiFi SSID=%s rssi=%d channel=%d\n", where,
                reinterpret_cast<CChar*>(ap.ssid), ap.rssi, ap.primary);
        } else {
            printf("[Nayan] %s esp_wifi_sta_get_ap_info failed\n", where);
        }
        fflush(stdout);
    }

    Private Static Void NayanLogPemSummary(CChar* label, const StdString& pem) {
        const Bool empty = pem.empty();
        const Bool hasBegin = !empty && pem.find("BEGIN") != StdString::npos;
        printf("[Nayan] PEM %s len=%u empty=%d has_BEGIN=%d\n",
            label, (unsigned)pem.size(), empty ? 1 : 0, hasBegin ? 1 : 0);
        if (!empty) {
            printf("[Nayan] PEM %s head=%.40s\n", label, pem.c_str());
        }
        fflush(stdout);
    }

    Private Static Void NayanLogDns(CChar* host, CChar* portStr) {
        printf("[Nayan] DNS start host=%s port=%s\n", host, portStr);
        fflush(stdout);
        struct addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo* res = nullptr;
        const TickType_t t0 = xTaskGetTickCount();
        const int gai = getaddrinfo(host, portStr, &hints, &res);
        const unsigned long ms = (unsigned long)((xTaskGetTickCount() - t0) * portTICK_PERIOD_MS);
        if (gai != 0) {
            printf("[Nayan] DNS FAIL rc=%d elapsed_ms=%lu (lwIP has no gai_strerror)\n",
                gai, ms);
            fflush(stdout);
            return;
        }
        int n = 0;
        for (struct addrinfo* p = res; p && n < 4; p = p->ai_next, ++n) {
            char ip[INET_ADDRSTRLEN] = {};
            struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
            inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));
            printf("[Nayan] DNS OK[%d] %s elapsed_ms=%lu\n", n, ip, ms);
        }
        if (res) {
            freeaddrinfo(res);
        }
        fflush(stdout);
    }

    Private Static Void NayanLogTcpProbe(CChar* host, Int port) {
        char portStr[8];
        snprintf(portStr, sizeof(portStr), "%d", port);
        struct addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo* res = nullptr;
        if (getaddrinfo(host, portStr, &hints, &res) != 0 || !res) {
            printf("[Nayan] TCP probe DNS failed host=%s\n", host);
            fflush(stdout);
            return;
        }
        const int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0) {
            printf("[Nayan] TCP probe socket() failed errno=%d\n", errno);
            freeaddrinfo(res);
            fflush(stdout);
            return;
        }
        struct timeval tv{};
        tv.tv_sec = 15;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        char ip[INET_ADDRSTRLEN] = {};
        struct sockaddr_in* sa = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
        inet_ntop(AF_INET, &sa->sin_addr, ip, sizeof(ip));
        const TickType_t t0 = xTaskGetTickCount();
        const int cr = connect(sock, res->ai_addr, res->ai_addrlen);
        const unsigned long ms = (unsigned long)((xTaskGetTickCount() - t0) * portTICK_PERIOD_MS);
        printf("[Nayan] TCP probe connect %s:%s -> rc=%d errno=%d elapsed_ms=%lu\n",
            ip, portStr, cr, cr == 0 ? 0 : errno, ms);
        close(sock);
        freeaddrinfo(res);
        fflush(stdout);
    }

    Private Static Void NayanLogTlsProbe(CChar* host, Int port,
            const StdString& claimCert, const StdString& claimKey) {
        printf("[Nayan] TLS mutual-auth probe host=%s port=%d TLS1.2 only\n", host, port);
        fflush(stdout);
        NayanLogWifiAndHeap("TLS probe before");

        esp_tls_cfg_t cfg = {};
        cfg.timeout_ms = kEnrollmentNetworkTimeoutMs;
        cfg.tls_version = ESP_TLS_VER_TLS_1_2;
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
        cfg.crt_bundle_attach = esp_crt_bundle_attach;
#else
        cfg.cacert_buf = nullptr;
#endif
        cfg.clientcert_buf = reinterpret_cast<const unsigned char*>(claimCert.c_str());
        cfg.clientcert_bytes = claimCert.size() + 1;
        cfg.clientkey_buf = reinterpret_cast<const unsigned char*>(claimKey.c_str());
        cfg.clientkey_bytes = claimKey.size() + 1;

        esp_tls_t* tls = esp_tls_init();
        if (!tls) {
            printf("[Nayan] TLS probe esp_tls_init failed\n");
            fflush(stdout);
            return;
        }

        const TickType_t t0 = xTaskGetTickCount();
        const int fd = esp_tls_conn_new_sync(host, static_cast<int>(strlen(host)), port, &cfg, tls);
        const unsigned long ms = (unsigned long)((xTaskGetTickCount() - t0) * portTICK_PERIOD_MS);
        if (fd >= 0) {
            printf("[Nayan] TLS probe OK fd=%d elapsed_ms=%lu\n", fd, ms);
        } else {
            esp_tls_last_error lastErr{};
            esp_tls_error_handle_t errHandle = &lastErr;
            if (esp_tls_get_error_handle(tls, &errHandle) == ESP_OK && errHandle) {
                printf("[Nayan] TLS probe FAIL fd=%d elapsed_ms=%lu esp_err=%d (%s) mbedtls=0x%x flags=0x%x\n",
                    fd, ms,
                    (int)errHandle->last_error,
                    esp_err_to_name(errHandle->last_error),
                    (unsigned)errHandle->esp_tls_error_code,
                    (unsigned)errHandle->esp_tls_flags);
            } else {
                printf("[Nayan] TLS probe FAIL fd=%d elapsed_ms=%lu\n", fd, ms);
            }
        }
        esp_tls_conn_destroy(tls);
        NayanLogWifiAndHeap("TLS probe after");
        fflush(stdout);
    }

    Private Static Void ParseBrokerHostPort(const StdString& uri, StdString& host, Int& port) {
        port = 8883;
        host.clear();
        const size_t scheme = uri.find("://");
        const size_t hostStart = (scheme == StdString::npos) ? 0 : scheme + 3;
        const size_t colon = uri.find(':', hostStart);
        const size_t slash = uri.find('/', hostStart);
        if (colon != StdString::npos && (slash == StdString::npos || colon < slash)) {
            host = uri.substr(hostStart, colon - hostStart);
            port = atoi(uri.c_str() + colon + 1);
        } else if (slash != StdString::npos) {
            host = uri.substr(hostStart, slash - hostStart);
        } else {
            host = uri.substr(hostStart);
        }
    }

    Private Void StartMqttClient() {
        printf("[Nayan] StartMqttClient enter mqttStarted=%d\n", mqttStarted ? 1 : 0);
        fflush(stdout);
        if (mqttStarted) {
            printf("[Nayan] StartMqttClient skip (already started)\n");
            fflush(stdout);
            return;
        }
        mqttStarted = true;
        enrollmentStarted = false;
        pendingSubscriptions = 0;

        enrollmentBrokerUri = NormalizeMqttUri(fpProfile.mqttEndpoint);
        Int brokerPort = 8883;
        ParseBrokerHostPort(enrollmentBrokerUri, enrollmentBrokerHost, brokerPort);
        enrollmentClientId = deviceService->GetSerialNumber();

        printf("[Nayan] broker uri_norm=%s host=%s port=%d client_id=%s timeout_ms=%d\n",
            enrollmentBrokerUri.c_str(), enrollmentBrokerHost.c_str(), brokerPort,
            enrollmentClientId.c_str(), kEnrollmentNetworkTimeoutMs);
        fflush(stdout);
        NayanLogWifiAndHeap("StartMqttClient");
        NayanLogPemSummary("fleet_ca", fpProfile.caCertificatePem);
        NayanLogPemSummary("fleet_claim_cert", fpProfile.clientCertificatePem);
        NayanLogPemSummary("fleet_claim_key", fpProfile.clientPrivateKeyPem);
        char portStr[8];
        snprintf(portStr, sizeof(portStr), "%d", brokerPort);
        NayanLogDns(enrollmentBrokerHost.c_str(), portStr);
        NayanLogTcpProbe(enrollmentBrokerHost.c_str(), brokerPort);
        NayanLogTlsProbe(enrollmentBrokerHost.c_str(), brokerPort,
            fpProfile.clientCertificatePem, fpProfile.clientPrivateKeyPem);

        esp_mqtt_client_config_t mqtt_cfg = {};
        mqtt_cfg.broker.address.hostname = enrollmentBrokerHost.c_str();
        mqtt_cfg.broker.address.port = brokerPort;
        mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_SSL;
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
        mqtt_cfg.broker.verification.crt_bundle_attach = esp_crt_bundle_attach;
        mqtt_cfg.broker.verification.certificate = nullptr;
        printf("[Nayan] mqtt broker verify=crt_bundle (saves ~1.2KB CA heap)\n");
#else
        mqtt_cfg.broker.verification.certificate = fpProfile.caCertificatePem.c_str();
        mqtt_cfg.broker.verification.certificate_len = fpProfile.caCertificatePem.size() + 1;
#endif
        mqtt_cfg.credentials.client_id = enrollmentClientId.c_str();
        mqtt_cfg.credentials.authentication.certificate = fpProfile.clientCertificatePem.c_str();
        mqtt_cfg.credentials.authentication.certificate_len =
            fpProfile.clientCertificatePem.size() + 1;
        mqtt_cfg.credentials.authentication.key = fpProfile.clientPrivateKeyPem.c_str();
        mqtt_cfg.credentials.authentication.key_len = fpProfile.clientPrivateKeyPem.size() + 1;
        mqtt_cfg.network.disable_auto_reconnect = true;
        mqtt_cfg.network.timeout_ms = kEnrollmentNetworkTimeoutMs;
        mqtt_cfg.buffer.size = 4096;
        mqtt_cfg.buffer.out_size = 4096;
        mqtt_cfg.task.stack_size = 20480;
        printf("[Nayan] mqtt buffers=4096 task_stack=%d\n", mqtt_cfg.task.stack_size);
        fflush(stdout);

        printf("[Nayan] cfg pointers hostname=%p ca=%p claim_cert=%p claim_key=%p client_id=%p\n",
            (void*)mqtt_cfg.broker.address.hostname,
            (void*)mqtt_cfg.broker.verification.certificate,
            (void*)mqtt_cfg.credentials.authentication.certificate,
            (void*)mqtt_cfg.credentials.authentication.key,
            (void*)mqtt_cfg.credentials.client_id);
        fflush(stdout);

        // Allow DNS/route to AWS IoT to settle (internet check does not validate *.amazonaws.com).
        printf("[Nayan] delay 2000ms before mqtt init\n");
        fflush(stdout);
        vTaskDelay(pdMS_TO_TICKS(2000));

        printf("[Nayan] esp_mqtt_client_init...\n");
        fflush(stdout);
        mqttClient = esp_mqtt_client_init(&mqtt_cfg);
        if (!mqttClient) {
            printf("[Nayan] esp_mqtt_client_init FAILED\n");
            fflush(stdout);
            logger->Error(Tag::Untagged,
                "Enrollment MQTT init failed host=" + enrollmentBrokerHost);
            mqttStarted = false;
            return;
        }
        printf("[Nayan] esp_mqtt_client_init OK handle=%p\n", (void*)mqttClient);
        fflush(stdout);
        esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_ANY, MqttEventHandler, this);
        printf("[Nayan] esp_mqtt_client_start...\n");
        fflush(stdout);
        const esp_err_t startErr = esp_mqtt_client_start(mqttClient);
        printf("[Nayan] esp_mqtt_client_start rc=%d (%s)\n",
            (int)startErr, esp_err_to_name(startErr));
        fflush(stdout);
        if (startErr != ESP_OK) {
            logger->Error(Tag::Untagged,
                "Enrollment MQTT start failed host=" + enrollmentBrokerHost);
            esp_mqtt_client_destroy(mqttClient);
            mqttClient = nullptr;
            mqttStarted = false;
            return;
        }

        logger->Info(Tag::Untagged,
            "MQTT client started for enrollment host=" + enrollmentBrokerHost
            + " port=" + std::to_string(brokerPort)
            + " client_id=" + enrollmentClientId
            + " timeout_ms=" + std::to_string(kEnrollmentNetworkTimeoutMs));
    }

    Private Static Void LogEnrollmentMqttError(FleetProvisioningService* self,
                                               esp_mqtt_event_handle_t event) {
        printf("[Nayan] MQTT_EVENT_ERROR handler enter\n");
        fflush(stdout);
        if (!event || !event->error_handle) {
            printf("[Nayan] MQTT_EVENT_ERROR no error_handle event=%p\n", (void*)event);
            fflush(stdout);
            self->logger->Error(Tag::Untagged, "MQTT error during enrollment (no error_handle)");
            return;
        }
        const esp_mqtt_error_codes_t* err = event->error_handle;
        printf("[Nayan] MQTT_EVENT_ERROR type=%d esp_err=%d (%s) mbedtls=%d cert_flags=0x%x sock_errno=%d\n",
            (int)err->error_type,
            (int)err->esp_tls_last_esp_err,
            esp_err_to_name(err->esp_tls_last_esp_err),
            err->esp_tls_stack_err,
            err->esp_tls_cert_verify_flags,
            err->esp_transport_sock_errno);
        fflush(stdout);
        NayanLogWifiAndHeap("MQTT_EVENT_ERROR");
        StdString line = "[Enrollment] MQTT transport error type="
            + std::to_string(static_cast<Int>(err->error_type))
            + " esp_err=" + std::to_string(static_cast<Int>(err->esp_tls_last_esp_err))
            + " mbedtls=" + std::to_string(err->esp_tls_stack_err)
            + " cert_flags=0x" + std::to_string(err->esp_tls_cert_verify_flags)
            + " sock_errno=" + std::to_string(err->esp_transport_sock_errno);
        if (err->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            line += " connect_return_code="
                + std::to_string(static_cast<Int>(err->connect_return_code));
        }
        self->logger->Error(Tag::Untagged, line);
    }

    Private Void SubscribeEnrollmentTopics() {
        pendingSubscriptions = 4;
        esp_mqtt_client_subscribe(mqttClient, fpProfile.createKeysAcceptedTopic.c_str(), 1);
        esp_mqtt_client_subscribe(mqttClient, fpProfile.createKeysRejectedTopic.c_str(), 1);
        esp_mqtt_client_subscribe(mqttClient, fpProfile.provisionAcceptedTopic.c_str(), 1);
        esp_mqtt_client_subscribe(mqttClient, fpProfile.provisionRejectedTopic.c_str(), 1);
        logger->Info(Tag::Untagged, "Subscribed to fleet provisioning topics");
    }

    Private Void PublishCreateKeysAndCertificate() {
        logger->Info(Tag::Untagged, "Publishing CreateKeysAndCertificate...");
        esp_mqtt_client_publish(mqttClient, fpProfile.createKeysRequestTopic.c_str(), "{}", 2, 1, 0);
    }

    Private Void HandleCreateKeysAccepted(CChar* payload, Int len) {
        std::lock_guard<std::mutex> lock(mutex_);
        logger->Info(Tag::Untagged, "CreateKeysAndCertificate ACCEPTED");

        StdString payloadStr(payload, len);

        if (!json_extract_string(payloadStr.c_str(), "certificateOwnershipToken", ownershipBuf, sizeof(ownershipBuf))) {
            logger->Error(Tag::Untagged, "Missing certificateOwnershipToken");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificateId", certIdBuf, sizeof(certIdBuf))) {
            logger->Error(Tag::Untagged, "Missing certificateId");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "privateKey", keyEscBuf, sizeof(keyEscBuf))) {
            logger->Error(Tag::Untagged, "Missing privateKey");
            return;
        }
        if (!json_extract_string(payloadStr.c_str(), "certificatePem", certEscBuf, sizeof(certEscBuf))) {
            logger->Error(Tag::Untagged, "Missing certificatePem");
            return;
        }

        unescape_json_to_pem(keyEscBuf, keyPemBuf, sizeof(keyPemBuf));
        unescape_json_to_pem(certEscBuf, certPemBuf, sizeof(certPemBuf));

        ownershipToken      = ownershipBuf;
        devicePrivateKeyPem = keyPemBuf;
        awsDeviceCertPem    = certPemBuf;

        logger->Info(Tag::Untagged, "certificateId: " + StdString(certIdBuf));
        logger->Info(Tag::Untagged, "certificateOwnershipToken: " + ownershipToken);

        PublishProvisionRequest();
    }

    Private Void PublishProvisionRequest() {
        StdString json = "{\"certificateOwnershipToken\":\"" + ownershipToken +
                         "\",\"parameters\":{\"SerialNumber\":\"" +
                         deviceService->GetSerialNumber() + "\"}}";
        logger->Info(Tag::Untagged, "Publishing RegisterThing for serial=" +
                                    deviceService->GetSerialNumber());
        esp_mqtt_client_publish(mqttClient, fpProfile.provisionRequestTopic.c_str(),
                                json.c_str(), static_cast<Int>(json.size()), 1, 0);
    }

    Private Void HandleProvisionAccepted(CChar* payload, Int len) {
        StdString payloadStr(payload, len);

        Char thingNameBuf[128];
        if (!json_extract_string(payloadStr.c_str(), "thingName", thingNameBuf, sizeof(thingNameBuf))) {
            logger->Error(Tag::Untagged, "Missing thingName in provision response");
            return;
        }

        Char tenantIdBuf[128];
        if (!json_extract_string(payloadStr.c_str(), "tenantId", tenantIdBuf, sizeof(tenantIdBuf))) {
            logger->Error(Tag::Untagged, "Missing tenantId in provision response");
            return;
        }

        logger->Info(Tag::Untagged, "========== ENROLLMENT SUCCESS ==========");
        logger->Info(Tag::Untagged, "thingName: " + StdString(thingNameBuf));
        logger->Info(Tag::Untagged, "SerialNumber: " + deviceService->GetSerialNumber());
        logger->Info(Tag::Untagged, "tenantId: " + StdString(tenantIdBuf));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            pendingTenantId = StdString(tenantIdBuf);
            thingName = StdString(thingNameBuf);
        }

        // SPIFFS + JSON serialize must not run on mqtt_task; use dedicated task with ample stack.
        if (xTaskCreate(EnrollmentCompleteTask, "enroll_save", 12288, this, 5, nullptr) != pdPASS) {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Failed_Provision;
            logger->Error(Tag::Untagged, "Failed to schedule credential save task");
        }
    }

    Private Void SaveReceivedCredentials(const StdString& tenantId) {
        logger->Info(Tag::Untagged, "Saving received credentials for tenantId: " + tenantId);

        DeviceIdentityProfileDto identityDto;
        identityDto.clientCertificatePem = Optional<StdString>(std::move(awsDeviceCertPem));
        identityDto.clientPrivateKeyPem  = Optional<StdString>(std::move(devicePrivateKeyPem));
        identityDto.thingName = Optional<StdString>(std::move(thingName));
        identityDto.tenantId = Optional<StdString>(tenantId);
        ownershipToken.clear();
        awsCaCertificatePem.clear();

        deviceService->SetDeviceIdentityProfile(identityDto);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            status = EnrollmentStatus::Success;
        }

        logger->Info(Tag::Untagged,
                     "Saved device identity profile for serial=" +
                     deviceService->GetSerialNumber());
    }

    Private Static Void MqttEventHandler(Void* handler_args, esp_event_base_t base,
                                         Int32 event_id, Void* event_data) {
        Var self = static_cast<FleetProvisioningService*>(handler_args);
        Var event = static_cast<esp_mqtt_event_handle_t>(event_data);
        const esp_mqtt_event_id_t ev = event
            ? event->event_id
            : static_cast<esp_mqtt_event_id_t>(event_id);
        printf("[Nayan] MqttEventHandler id=%ld (%s) base=%s\n",
            (long)ev, MqttEventIdName(static_cast<Int32>(ev)), base ? base : "null");
        fflush(stdout);

        switch (ev) {
            case MQTT_EVENT_BEFORE_CONNECT:
                printf("[Nayan] BEFORE_CONNECT host=%s client_id=%s\n",
                    self->enrollmentBrokerHost.c_str(),
                    self->enrollmentClientId.c_str());
                NayanLogWifiAndHeap("BEFORE_CONNECT");
                fflush(stdout);
                break;

            case MQTT_EVENT_CONNECTED:
                printf("[Nayan] MQTT_EVENT_CONNECTED OK\n");
                fflush(stdout);
                self->logger->Info(Tag::Untagged, "MQTT connected");
                self->SubscribeEnrollmentTopics();
                break;

            case MQTT_EVENT_SUBSCRIBED:
                if (--self->pendingSubscriptions == 0 && !self->enrollmentStarted) {
                    self->enrollmentStarted = true;
                    self->PublishCreateKeysAndCertificate();
                }
                break;

            case MQTT_EVENT_DATA: {
                StdString topic(event->topic, event->topic_len);

                if (topic == self->fpProfile.createKeysAcceptedTopic) {
                    self->HandleCreateKeysAccepted(event->data, event->data_len);
                } else if (topic == self->fpProfile.provisionAcceptedTopic) {
                    self->HandleProvisionAccepted(event->data, event->data_len);
                } else if (topic == self->fpProfile.provisionRejectedTopic) {
                    std::lock_guard<std::mutex> lock(self->mutex_);
                    self->status = EnrollmentStatus::Failed_Provision;
                    self->logger->Error(Tag::Untagged, "Provisioning rejected");
                }
                break;
            }

            case MQTT_EVENT_ERROR: {
                LogEnrollmentMqttError(self, event);
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->status = EnrollmentStatus::Failed_MqttConnect;
                self->mqttStarted = false;
                break;
            }

            case MQTT_EVENT_DISCONNECTED: {
                printf("[Nayan] MQTT_EVENT_DISCONNECTED status=%d\n",
                    (int)self->GetEnrollmentStatus());
                fflush(stdout);
                std::lock_guard<std::mutex> lock(self->mutex_);
                if (self->status == EnrollmentStatus::InProgress) {
                    self->status = EnrollmentStatus::Failed_MqttConnect;
                    self->logger->Error(Tag::Untagged, "MQTT disconnected during enrollment");
                }
                break;
            }

            default:
                printf("[Nayan] MqttEventHandler unhandled id=%ld\n", (long)ev);
                fflush(stdout);
                break;
        }
    }
};
#endif // FLEETPROVISIONINGSERVICE_H
