#ifndef WAKEUPINFO_H
#define WAKEUPINFO_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define ms_TO_S_FACTOR 1000    /* Conversion factor for milliseconds to seconds */

class WakeUpInfoClass
{
public:
    void begin();
    void setTimerWakeUp(uint32_t wakeupIn);
    void setSleepTime(uint32_t sleepIn);
    uint32_t getWakeupInterval();
    uint32_t getSleepTime();
    boolean isPoweredOn();

    void suspendSleep();
    void resumeSleep();
    void tick();

private:
    boolean _manualWakeup;
    boolean _isPowerReset;
    int _flag;
    char _wakeupReason[32];
    uint32_t _wakeupIn;
    uint32_t _sleepIn;
    uint64_t _last_check;
};

extern WakeUpInfoClass WakeUp;
#endif