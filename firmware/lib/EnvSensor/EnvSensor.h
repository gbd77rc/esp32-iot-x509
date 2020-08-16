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

    // static TaskHandle_t readTaskHandle;
    // static SemaphoreHandle_t semaphoreFlag;
    // static bool taskCreated;
    bool taskToRun() override;   

private:
    // void pageInfo();
    ScaleType _scale;
    float _humidity;
    float _temperature;
    float _pressure;
    bool _testOnly;
    uint8_t _id;
    byte _datos[5];
    // byte readDevice();
    // float readTemperature();
    // float readHumidity();
    char _symbol[2];
    uint64_t _last_read;
    uint8_t _address;
    bool _canRead;
};

extern EnvSensorClass EnvSensor;

#endif
