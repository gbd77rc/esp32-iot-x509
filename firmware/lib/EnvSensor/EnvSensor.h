#ifndef ENVSENSORS_H
#define ENVSENSORS_H

#include <Wire.h>
#include <Adafruit_BMP280.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "Config.h"

typedef enum
{
    ENV_CELSIUS = 1,
    ENV_KELVIN = 2,
    ENV_FAHRENHEIT = 3
} ScaleType;

class EnvSensorClass : public BaseConfigInfoClass
{
public:
    EnvSensorClass() : BaseConfigInfoClass("envSensor") {}

    static void readTask(void *parameters);
    static uint16_t resumeTask();

    void begin();
    bool read();
    void toJson(JsonObject ob) override;
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    bool isConnected();
    bool connect();
    void envInfo();
    void displayInfo();
    float getTemperature();
    const char *getSymbol();
    static TaskHandle_t readTaskHandle;
    static SemaphoreHandle_t semaphoreFlag;
    static bool taskCreated;

private:
    void pageInfo();
    ScaleType _scale;
    float _humidity;
    float _temperature;
    float _pressure;
    bool _testOnly;
    uint8_t _id;
    byte _datos[5];
    byte readDevice();
    float readTemperature();
    float readHumidity();
    char _symbol[2];
    uint64_t _last_read;
    uint8_t _address;
    bool _connected;
    bool _canRead;
};

extern EnvSensorClass EnvSensor;

#endif
