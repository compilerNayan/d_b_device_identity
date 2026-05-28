#ifndef CONNECTIONCONSTANTS_H
#define CONNECTIONCONSTANTS_H

#define FLEET_PROVISIONING_MQTT_ENDPOINT "fleet/provisioning/mqtt/endpoint"
#define FLEET_PROVISIONING_CA_CERTIFICATE_PEM "fleet/provisioning/ca/certificate.pem"
#define FLEET_PROVISIONING_CLIENT_CERTIFICATE_PEM "fleet/provisioning/client/certificate.pem"
#define FLEET_PROVISIONING_CLIENT_PRIVATE_KEY_PEM "fleet/provisioning/client/private/key.pem"

#define FLEET_PROVISIONING_CREATE_KEYS_REQUEST_TOPIC "create/keys/request"
#define FLEET_PROVISIONING_CREATE_KEYS_ACCEPTED_TOPIC "create/keys/accepted"
#define FLEET_PROVISIONING_CREATE_KEYS_REJECTED_TOPIC "create/keys/rejected"
#define FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC "provision/request"
#define FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC "provision/accepted"

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