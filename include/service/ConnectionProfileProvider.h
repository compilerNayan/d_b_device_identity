#ifndef TOPICNAMESTORE_H
#define TOPICNAMESTORE_H

#include <StandardDefines.h>

class ConnectionDefaults {

    Public Static StdString FLEET_PROVISIONING_MQTT_ENDPOINT = "mqtts://a2hlcpmplecdfa-ats.iot.us-east-1.amazonaws.com";
    Public Static StdString FLEET_PROVISIONING_TEMPLATE_NAME = "SomeTemplateName";
    Public Static StdString FLEET_PROVISIONING_CA_CERTIFICATE_PEM = "fleet/provisioning/ca/certificate.pem";
    Public Static StdString FLEET_PROVISIONING_CLIENT_CERTIFICATE_PEM = "fleet/provisioning/client/certificate.pem";
    Public Static StdString FLEET_PROVISIONING_CLIENT_PRIVATE_KEY_PEM = "fleet/provisioning/client/private/key.pem";
    
    Public Static StdString FLEET_PROVISIONING_CREATE_KEYS_REQUEST_TOPIC = "$aws/certificates/create/json";
    Public Static StdString FLEET_PROVISIONING_CREATE_KEYS_ACCEPTED_TOPIC = "$aws/certificates/create/json/accepted";
    Public Static StdString FLEET_PROVISIONING_CREATE_KEYS_REJECTED_TOPIC = "$aws/certificates/create/json/rejected";

    Private Static StdString FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC_PREFIX = "$aws/provisioning-templates/";
    Private Static StdString FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC_SUFIX = "/provision/json";
    Public Static StdString FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC = FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC_PREFIX + 
                                                                            FLEET_PROVISIONING_TEMPLATE_NAME + 
                                                                            FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC_SUFIX;

    Private Static StdString FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC_PREFIX = "$aws/provisioning-templates/";
    Private Static StdString FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC_SUFIX = "/provision/json/accepted";
    Public Static StdString FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC = FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC_PREFIX + 
                                                                           FLEET_PROVISIONING_TEMPLATE_NAME + 
                                                                           FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC_SUFIX;

    Private Static StdString FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC_PREFIX = "$aws/provisioning-templates/";
    Private Static StdString FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC_SUFIX = "/provision/json/rejected";
    Public Static StdString FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC = FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC_PREFIX + 
                                                                           FLEET_PROVISIONING_TEMPLATE_NAME + 
                                                                           FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC_SUFIX;
    Public Static Void SetThingName(StdString thingName) {
        THING_NAME = thingName;
        FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC = FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC_PREFIX + 
                        THING_NAME + 
                        FLEET_PROVISIONING_PROVISION_REQUEST_TOPIC_SUFIX;
        FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC = FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC_PREFIX + 
                        THING_NAME + 
                        FLEET_PROVISIONING_PROVISION_ACCEPTED_TOPIC_SUFIX;
        FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC = FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC_PREFIX + 
                        THING_NAME + 
                        FLEET_PROVISIONING_PROVISION_REJECTED_TOPIC_SUFIX;
    }

    Private Static StdString TENANT_ID = "123";
    Private Static StdString DEVICE_TYPE = "switch";
    Private Static StdString THING_NAME = "switch1";
    Private Static StdString CONNECTION_PREFIX = TENANT_ID + "/" + DEVICE_TYPE + "/" + THING_NAME;

    Public Static StdString DEVICE_IDENTITY_MQTT_ENDPOINT = FLEET_PROVISIONING_MQTT_ENDPOINT;
    Public Static StdString DEVICE_IDENTITY_CA_CERTIFICATE_PEM = FLEET_PROVISIONING_CA_CERTIFICATE_PEM;
    
    Public Static StdString DEVICE_IDENTITY_PUBLISH_TOPICS_STATUS_TOPIC = CONNECTION_PREFIX + "/status";
    Public Static StdString DEVICE_IDENTITY_PUBLISH_TOPICS_TELEMETRY_TOPIC = CONNECTION_PREFIX + "/telemetry";
    Public Static StdString DEVICE_IDENTITY_PUBLISH_TOPICS_LOGS_TOPIC = CONNECTION_PREFIX + "/logs";
    Public Static StdString DEVICE_IDENTITY_PUBLISH_TOPICS_EVENTS_TOPIC = CONNECTION_PREFIX + "/events";
    Public Static StdString DEVICE_IDENTITY_SUBSCRIBE_TOPICS_COMMAND_TOPIC = CONNECTION_PREFIX + "/command";
    Public Static StdString DEVICE_IDENTITY_SUBSCRIBE_TOPICS_OTA_UPDATE_TOPIC = CONNECTION_PREFIX + "/ota/update";
    Public Static StdString DEVICE_IDENTITY_SUBSCRIBE_TOPICS_FEATURE_FLAG_TOPIC = CONNECTION_PREFIX + "/feature/flag";
    
};
#endif