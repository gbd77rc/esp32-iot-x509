#include "LogInfo.h"
#include "WakeUpInfo.h"

RTC_DATA_ATTR int _bootCount = 0;
RTC_DATA_ATTR unsigned long _bootTime = 0;

void WakeUpInfoClass::begin()
{
    this->suspendSleep();
    this->_isPowerReset = false;
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    this->_manualWakeup = false;
    this->_flag = 0;
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        LogInfo.log(LOG_VERBOSE, F("Wakeup caused by external signal using RTC_IO"));
        this->_manualWakeup = true;
        strcpy(this->_wakeupReason, "ESP_SLEEP_WAKEUP_EXT0");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        LogInfo.log(LOG_VERBOSE, F("Wakeup caused by external signal using RTC_CNTL"));
        this->_manualWakeup = true;
        strcpy(this->_wakeupReason, "ESP_SLEEP_WAKEUP_EXT1");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        LogInfo.log(LOG_VERBOSE, F("Wakeup caused by timer"));
        strcpy(this->_wakeupReason, "ESP_SLEEP_WAKEUP_TIMER");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        LogInfo.log(LOG_VERBOSE, F("Wakeup caused by touchpad"));
        strcpy(this->_wakeupReason, "ESP_SLEEP_WAKEUP_TOUCHPAD");
        this->_manualWakeup = true;
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        LogInfo.log(LOG_VERBOSE, F("Wakeup caused by ULP program"));
        strcpy(this->_wakeupReason, "ESP_SLEEP_WAKEUP_ULP");
        break;
    default:
        LogInfo.log(LOG_INFO, F("Waked up because of power on or manual reset!"));
        strcpy(this->_wakeupReason, "ESP_SLEEP_WAKEUP_UNDEFINED");
        _bootTime = 0;
        this->_manualWakeup = true;
        this->_isPowerReset = false;
        break;
    }
    _bootCount++;
    this->resumeSleep();
}

void WakeUpInfoClass::setTimerWakeUp(uint32_t wakeupIn)
{
    this->_wakeupIn = wakeupIn;
    esp_sleep_enable_timer_wakeup(wakeupIn * uS_TO_S_FACTOR);
    LogInfo.log(LOG_VERBOSE, "Setup ESP32 to wake up after %i Seconds", wakeupIn);
}

uint32_t WakeUpInfoClass::getWakeupInterval()
{
    return this->_wakeupIn;
}

void WakeUpInfoClass::setSleepTime(uint32_t sleepIn)
{
    this->_sleepIn = sleepIn;
    LogInfo.log(LOG_VERBOSE, "Setup ESP32 to sleep in %i Seconds", sleepIn);
}

uint32_t WakeUpInfoClass::getSleepTime()
{
    return this->_sleepIn;
}

void WakeUpInfoClass::suspendSleep()
{
    LogInfo.log(LOG_INFO, F("Suspending Sleep..."));
    this->_flag++;
}

void WakeUpInfoClass::resumeSleep()
{
    this->_flag--;

    if (this->_flag < 0)
    {
        this->_flag = 0;
    }

    if (this->_flag == 0)
    {
        LogInfo.log(LOG_INFO, F("Resuming Sleep..."));
    }
}

bool WakeUpInfoClass::canSleep()
{
    return this->_flag == 0;
}

unsigned long WakeUpInfoClass::getAliveTime()
{
    return _bootTime;
}

int WakeUpInfoClass::getBootCount()
{
    return _bootCount;
}

boolean WakeUpInfoClass::isManualWakeUp()
{
    return this->_manualWakeup;
}

boolean WakeUpInfoClass::isPoweredOn()
{
    return this->_isPowerReset;
}

void WakeUpInfoClass::tick()
{
    if (this->canSleep())
    {
        if (((millis() - this->_last_check) / ms_TO_S_FACTOR) >= this->getSleepTime())
        {
            if (WakeUp.canSleep())
            {
                this->_last_check = millis();
                if (Configuration.shouldSave())
                {
                    Configuration.save();
                }
                LogInfo.log(LOG_INFO, F("Going to sleep now"));
                _bootTime += millis();
                LogInfo.log(LOG_VERBOSE, "Been alive for %lu seconds", _bootTime / 1000);
                esp_deep_sleep_start();
            }
        } 
    }
}

uint64_t WakeUpInfoClass::getBootTime()
{
    return _bootTime + millis();
}

WakeUpInfoClass WakeUp;