#ifndef LEDINFO_H
#define LEDINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "Config.h"

#define LED_COUNT 3
typedef enum
{
    LED_POWER = 0,
    LED_LIDAR = 1,
    LED_GPS = 2,
} LedType;

class LedInfoClass : public BaseConfigInfoClass
{
public:
    LedInfoClass() : BaseConfigInfoClass("ledInfo") {}

    void begin();
    void toJson(JsonObject ob) override;
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;

    void switchOn(LedType type);
    void switchOff(LedType type);

    bool setIsEnabled(bool flag);
    
private:
    bool _isEnabled;
    uint8_t _pins[LED_COUNT];
    void initialise();
};

extern LedInfoClass LedInfo;


#endif