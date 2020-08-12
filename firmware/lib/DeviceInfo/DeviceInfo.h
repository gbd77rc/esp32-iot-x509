#ifndef SYSINFO_h
#define SYSINFO_h

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#include "Config.h"

class DeviceInfoClass : public BaseConfigInfoClass
{
public:
    DeviceInfoClass() : BaseConfigInfoClass("device") {}
    void begin();
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    void toJson(JsonObject ob) override;
    const char *deviceId();
    bool setLocation(const char *newLocation);
    const char *location();

private:
    char _prefix[20];
    char _device_id[32];
    char _location[64];
    int _wakeupTime;
};

extern DeviceInfoClass DeviceInfo;
#endif
