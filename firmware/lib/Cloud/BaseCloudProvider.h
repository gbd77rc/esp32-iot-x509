#ifndef BASECLOUDPROVIDER_H
#define BASECLOUDPROVIDER_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include "CloudMisc.h"

const uint8_t QOS_LEVEL = 0;
const uint8_t RECONNECT_RETRIES = 5;

class BaseCloudProvider;

typedef struct cloudInstanceStruct
{
    BaseCloudProvider *instance;
    TaskHandle_t checkTaskHandle;
} CloudInstance;

class BaseCloudProvider
{
public:
    static void checkTask(void *parameters);
    static void connectTask(void *parameters);

    BaseCloudProvider(CloudProviderType type);
    void begin(DATABUILDER builder);
    bool virtual connect(const IoTConfig *config) = 0;
    void virtual processReply(char *topic, byte *payload, unsigned int length) = 0;
    bool sendData();
    bool canSendNow();
    bool getIsConnected();
    const char* getProviderType();
    void tick();
    const SemaphoreHandle_t getSemaphore();


protected:
    void virtual loadTopics() = 0;
    void initialiseConnection(std::function<void (char *, uint8_t *, unsigned int)> callback);
    bool mqttConnection();
    void virtual buildUserName(char *userName) = 0;
    void virtual processDesiredStatus(JsonObject doc) = 0;
    bool virtual updateProperty(const char *property, JsonVariant value);
    bool virtual sendDeviceReport(JsonObject json);
    bool sendTelemetry(JsonObject json);
    const char* getFirstTopic(TopicType type);
    void checkForMessages();
    WiFiClientSecure _httpsClient;
    PubSubClient _mqttClient;
    CloudProviderType _providerType;
    const IoTConfig *_config;
    bool _connected;
    uint8_t _retries;
    bool _tryConnecting;
    IOTTOPIC _topics[6];
    uint8_t _topicsAdded;
    CloudInstance _cloudInstance;
    uint64_t _lastSent;
    char _buffer[64];
    DATABUILDER _builder;    
};

#endif