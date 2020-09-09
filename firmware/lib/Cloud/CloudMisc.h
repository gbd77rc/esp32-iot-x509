#ifndef CLOUDMISC_H
#define CLOUDMISC_H

#include <Arduino.h>

#define CERT_COUNT 3

typedef enum
{
    CT_CERT = 0,
    CT_KEY = 1,
    CT_CA = 2
} CertType;

typedef enum
{
    CPT_AZURE = 0,
    CPT_AWS = 1,
    CPT_UNKNOWN =2
} CloudProviderType;

typedef struct CertificateInfo
{
    char fileName[32];
    CertType type;
    char *contents;
} CERTIFICATE;

typedef struct IoTConfig
{
    CloudProviderType provider;
    char endPoint[256];
    uint32_t port;
    bool sendTelemetry;
    bool sendDeviceTwin;
    uint16_t sendInterval;
    CERTIFICATE certificates[CERT_COUNT];
    SemaphoreHandle_t semaphore;    
} IOTCONFIG;

typedef enum{
    TT_UNKNOWN,
    TT_SUBSCRIBE,
    TT_DEVICETWIN,
    TT_SYNCDEVICETWIN,
    TT_TELEMETRY
} TopicType;

typedef struct IoTTopic
{
    char topic[128];
    TopicType type;
    bool appendUniqueId;
} IOTTOPIC;

typedef void (*DATABUILDER)(JsonObject payload, bool isDeviceTwin);
typedef void (*DESIREDPROCESSOR)(JsonObject payload);

#endif