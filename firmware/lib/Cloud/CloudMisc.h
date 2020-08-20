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
    CPT_AWS = 1
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
    char hubName[64];
    uint32_t port;
    bool sendTelemetry;
    uint16_t sendInterval;
    CERTIFICATE certificates[CERT_COUNT];
} IOTCONFIG;

#endif