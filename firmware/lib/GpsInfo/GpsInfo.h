#ifndef GPSINFO_H
#define GPSINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <SIM808.h>
#include <TinyGPS++.h>
//#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include "Config.h"


class GpsInfoClass : public BaseConfigInfoClass
{
public:
    GpsInfoClass() : BaseConfigInfoClass("gpsInfo"), _gpsSerial(2), _sim(13) {}

    static void readTask(void *parameters);
    static uint16_t resumeTask();
    static TaskHandle_t readTaskHandle;
    static SemaphoreHandle_t semaphoreFlag;
    static bool taskCreated;
    static long lastCheck;      

    void begin();
    bool read();
    void toJson(JsonObject ob) override;
    void toGeoJson(JsonObject ob);
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    bool isConnected();
    bool connect();
    float getLong();
    float getLat();
    uint16_t getSatelites();
    const char *location();

private:
    bool _isConnected;
    uint32_t _count;
    long _last_read;
    uint16_t _txPin;
    uint16_t _rxPin;
    uint32_t _baud;
    char _location[65];
    bool _isSim808;

    float _long;
    float _lat;
    uint16_t _satelites;
    uint16_t _course;
    uint16_t _speed;
    float _altitude;
    bool _isValid;

    bool _isEnabled;
    //SoftwareSerial _gpsSerial;
    HardwareSerial _gpsSerial;
    SIM808 _sim;
};

extern GpsInfoClass GpsInfo;

#endif