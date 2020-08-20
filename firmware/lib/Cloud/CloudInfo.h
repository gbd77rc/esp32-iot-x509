#ifndef BASECLOUD_H
#define BASECLOUD_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "Config.h"

#define CERT_COUNT 4
typedef enum
{
    CT_CERT = 0,
    CT_KEY = 1,
    CT_CA_AZURE = 2,
    CT_CA_AWS = 3,
} CertType;

typedef struct CertificateInfo
{
    char fileName[32];
    CertType type;
    char *contents;
} CERTIFICATE;

typedef struct IoTConfig
{
    char cloudInstance[10];
    char endPoint[256];
    char hubName[64];
    uint32_t port;
    bool sendTelemetry;
    uint16_t sendInterval;
    CERTIFICATE certificates[CERT_COUNT];
} IOTCONFIG;


class BaseCloudInfo
{
    /**
     * Connect to the correct cloud
     */    
    void virtual connect() = 0;
};

class CloudInfoClass: public  BaseConfigInfoClass
{
public:
    CloudInfoClass();

    void begin();
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    void toJson(JsonObject ob) override;    
    char *getCloudInstanceName();

private:
    IOTCONFIG _config;
    PubSubClient _mqttClient;    
};

extern CloudInfoClass CloudInfo;

// "endpoint": "",
// "name": "",
// "port": 8883,
// "sendTelemetry": true,
// "intervalSeconds": 45

#endif