#ifndef GPSINFO_H
#define GPSINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "Config.h"
#include "BaseSensor.h"


class GpsInfoClass : public BaseConfigInfoClass, public BaseSensorClass
{
public:
    GpsInfoClass() : BaseConfigInfoClass("gpsSensor"), BaseSensorClass("gps"), _gpsSerial(2) {}  

    void begin(SemaphoreHandle_t flag) override;
    void toJson(JsonObject ob) override;
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    const bool connect() override;
    bool taskToRun() override;   
    const char* toString() override;

    void toGeoJson(JsonObject ob);
    void changeEnabled(bool flag) override;

private:
    uint16_t _txPin;
    uint16_t _rxPin;
    uint32_t _baud;
    char _location[65];

    float _long;
    float _lat;
    uint16_t _satelites;
    uint16_t _course;
    uint16_t _speed;
    float _altitude;
    bool _isValid;
    HardwareSerial _gpsSerial;
};

extern GpsInfoClass GpsSensor;

#endif