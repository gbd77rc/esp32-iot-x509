#ifndef GPSINFO_H
#define GPSINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <TinyGPS++.h>
//#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include "Config.h"

class GpsInfoClass : public BaseConfigInfoClass
{
public:
    GpsInfoClass() : BaseConfigInfoClass("gpsInfo"), _gpsSerial(2) {}

    static void readTask(void *parameters);
    static uint16_t resumeTask();
    static TaskHandle_t readTaskHandle;
    static SemaphoreHandle_t semaphoreFlag;
    static bool taskCreated;

    void begin();
    bool read();
    void toJson(JsonObject ob) override;
    void toGeoJson(JsonObject ob);
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    bool isConnected();
    bool connect();
    const char *location();

private:
    bool _isConnected;
    uint32_t _count;
    long _last_read;
    uint16_t _txPin;
    uint16_t _rxPin;
    uint32_t _baud;
    char _location[65];

    bool _isEnabled;
    TinyGPSPlus _gps;
    //SoftwareSerial _gpsSerial;
    HardwareSerial _gpsSerial;
};

extern GpsInfoClass GpsInfo;

#endif