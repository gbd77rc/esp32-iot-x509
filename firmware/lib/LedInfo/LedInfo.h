#ifndef LEDINFO_H
#define LEDINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "Config.h"

#define LED_COUNT 3
typedef enum
{
    LED_POWER = 0,
    LED_WIFI= 1,
    LED_CLOUD = 2,
} LedType;

typedef struct ledStateStruct {
    char typeName[10];
    uint8_t idx;
    uint8_t pin;
    bool isOn;
    uint8_t brightness;
} LedState;

class LedInfoClass : public BaseConfigInfoClass
{
public:
    LedInfoClass() : BaseConfigInfoClass("ledInfo") {}

    static void blinkTask(void *parameters);
    static SemaphoreHandle_t semaphoreFlag;
    static TaskHandle_t blinkTaskHandles[];    

    void begin(){};
    void toJson(JsonObject ob) override;
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;

    void switchOn(LedType type);
    void switchOff(LedType type);
    void blinkOn(LedType type);
    void blinkOff(LedType type);
    void setBrightness(uint8_t brightness);
    
private:
    bool _isEnabled;
    LedState _led[LED_COUNT];
    uint8_t _brightness;
    void initialise();
    const char* ledTypeToString(LedType level);
};

extern LedInfoClass LedInfo;


#endif