#ifndef ENVSENSORS_H
#define ENVSENSORS_H

#include <SimpleDHT.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "BaseSensor.h"
#include "Config.h"

typedef enum
{
    ENV_CELSIUS = 1,
    ENV_KELVIN = 2,
    ENV_FAHRENHEIT = 3
} ScaleType;

class EnvSensorClass : public BaseConfigInfoClass, public BaseSensorClass
{
public:
    EnvSensorClass() : BaseConfigInfoClass("envSensor"), BaseSensorClass("env") {}

    void begin(SemaphoreHandle_t flag) override;
    void toJson(JsonObject ob) override;
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    const bool connect() override;
    bool taskToRun() override;   
    const char* toString() override;
    const char* getSymbol();

private:
    ScaleType _scale;
    float _humidity;
    float _temperature;
    uint8_t _dataPin;
    SimpleDHT22 _sensor;
    long _epoch_time;
};

extern EnvSensorClass EnvSensor;

#endif
