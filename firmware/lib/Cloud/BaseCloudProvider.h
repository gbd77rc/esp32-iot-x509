#ifndef BASECLOUDPROVIDER_H
#define BASECLOUDPROVIDER_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "CloudMisc.h"

const uint8_t QOS_LEVEL = 0;
const uint8_t RECONNECT_RETRIES = 5;

class BaseCloudProvider
{
public:
    BaseCloudProvider(CloudProviderType type);

    bool virtual connect(const IoTConfig *config) = 0;
    bool getIsConnected();

protected:
    void initialiseConnection(std::function<void (char *, uint8_t *, unsigned int)> callback);
    WiFiClientSecure _httpsClient;
    PubSubClient _mqttClient;
    CloudProviderType _providerType;
    const IoTConfig *_config;
    bool _connected;
    uint8_t _retries;
    bool _tryConnecting;
    char _twinDelta[64];
    char _twinResponse[64];
    char _deviceEvents[64];
    char _telemetry[64];
};

#endif