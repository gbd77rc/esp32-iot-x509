#ifndef WIFIINFO_h
#define WIFIINFO_h

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <WiFi.h>
#include "Display.h"

class WiFiInfoClass
{
public:
    void begin();
    void toJson(JsonObject obj);    
    const char* getSSID();
    bool connect(u8g2_uint_t x, u8g2_uint_t y);
    const bool getIsConnected();

private:
    void wpsInitConfig();
    static void WiFiEvent(WiFiEvent_t event, system_event_info_t info);
    static String numbersToString(uint8_t a[]);
    unsigned long previousMillisWiFi;
    const long intervalWiFi = 6000;
    bool _connected;
    char _ssid[32];
};

extern WiFiInfoClass WiFiInfo;

#endif