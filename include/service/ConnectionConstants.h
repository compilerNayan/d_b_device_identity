#ifndef CONNECTIONCONSTANTS_H
#define CONNECTIONCONSTANTS_H

#include <StandardDefines.h>




static const char *const TOPIC_CREATE_ACCEPTED =
    "";
static const char *const TOPIC_CREATE_REJECTED =
    "";
static const char *const TOPIC_CREATE_REQUEST =
    "";





#define DEVICE_IDENTITY_MQTT_ENDPOINT "device/identity/mqtt/endpoint"
#define DEVICE_IDENTITY_CA_CERTIFICATE_PEM "device/identity/ca/certificate.pem"
#define DEVICE_IDENTITY_PUBLISH_TOPICS_STATUS_TOPIC "device/identity/publish/topics/status"
#define DEVICE_IDENTITY_PUBLISH_TOPICS_TELEMETRY_TOPIC "device/identity/publish/topics/telemetry"
#define DEVICE_IDENTITY_PUBLISH_TOPICS_LOGS_TOPIC "device/identity/publish/topics/logs"
#define DEVICE_IDENTITY_PUBLISH_TOPICS_EVENTS_TOPIC "device/identity/publish/topics/events"
#define DEVICE_IDENTITY_SUBSCRIBE_TOPICS_COMMAND_TOPIC "device/identity/subscribe/topics/command"
#define DEVICE_IDENTITY_SUBSCRIBE_TOPICS_OTA_UPDATE_TOPIC "device/identity/subscribe/topics/ota/update"
#define DEVICE_IDENTITY_SUBSCRIBE_TOPICS_FEATURE_FLAG_TOPIC "device/identity/subscribe/topics/feature/flag"

#endif