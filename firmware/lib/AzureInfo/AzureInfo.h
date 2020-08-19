#ifndef AZUREINFO_H
#define AZUREINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "Config.h"

const uint8_t QOS_LEVEL = 0;
const uint8_t RECONNECT_RETRIES = 5;

struct CertificateInfo
{
    char fileName[32];
    char *contents;
};

class AzureInfoClass : public BaseConfigInfoClass
{
public:
    static void checkTask(void *parameters);
    static void sendDeviceTask(void *parameters);
    static void sendTelemetryTask(void *parameters);
    static uint16_t resumeCheckTask();
    static uint16_t resumeSendDeviceTask();
    static uint16_t resumeSendTelemetryTask();

    AzureInfoClass();
    void begin();
    void toJson(JsonObject ob) override;
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    bool isConnected();
    bool canSend();
    bool connect();
    void checkForMessage();
    bool setCanSend(bool enabled);
    bool reportTwin(JsonObject json);
    bool sendTelemetry(JsonObject json);
    static void loadCertificate(CertificateInfo *cert);
    void desiredUpdate(char *topic, byte *payload, unsigned int length);
    bool getCurrentStatus();
    static bool menu(ezMenu *callingMenu);
    void page1();
    static TaskHandle_t checkTaskHandle;
    static TaskHandle_t sendDeviceTaskHandle;
    static TaskHandle_t sendTelemetryTaskHandle;
    static SemaphoreHandle_t semaphoreFlag;
    static bool taskCreated;

private:
    void processDesiredStatus(JsonObject doc);
    void processDesiredUpdate(JsonObject doc);
    PubSubClient _mqttClient;
    uint64_t _last_sent;
    uint32_t _msg_count;
    uint16_t _port;
    bool _connected;
    bool _canSend;
    char _hub_connection[256];
    char _telemetry[64];
    char _hub_name[64];
    CertificateInfo _certs[3];
    char _deviceTwin[64];
    char _deviceEvents[64];
    void buildUserName(char *userName);
    bool updateProperty(const char *property, JsonVariant value);
    bool _tryConnecting;
};

extern AzureInfoClass AzureInfo;

#endif
