#ifndef LIDARINFO_H
#define LIDARINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <TFmini_plus.h>
#include "BaseSensor.h"
#include "Config.h"

class LiDARInfoClass : public BaseConfigInfoClass, public BaseSensorClass
{
public:
    LiDARInfoClass() : BaseConfigInfoClass("lidarInfo"), BaseSensorClass("lidar"), _lidarSerial(1) {}
    // static void readTask(void *parameters);
    // static uint16_t resumeTask();    
    // static TaskHandle_t readTaskHandle;
    // static SemaphoreHandle_t semaphoreFlag;
    // static bool taskCreated;
    // static long lastCheck;    

    void begin(SemaphoreHandle_t flag) override;
    bool read();
    void toJson(JsonObject ob) override;
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override; 
    const bool connect() override;    
    uint16_t getDistance();
    const char* toString() override;
    bool taskToRun() override;  

private:
    bool _isEnabled;
    long _last_read;
    uint16_t _txPin;
    uint16_t _rxPin;
    uint32_t _baud;
    HardwareSerial _lidarSerial;
    TFminiPlus _lidar;
    tfminiplus_data_t _data;
    bool _isValid;
};

extern LiDARInfoClass LiDARInfo;

#endif