#include "LogInfo.h"
#include "WakeUpInfo.h"

RTC_DATA_ATTR int _bootCount = 0;
RTC_DATA_ATTR unsigned long _bootTime = 0;

/**
 * Begin the initialization of the wakeup and sleep system
 */
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

/**
 * Set how long to wait before waking up from sleep in seconds
 * 
 * @param wakeupIN How many seconds to wait
 */
void WakeUpInfoClass::setTimerWakeUp(uint32_t wakeupIn)
{
    this->_wakeupIn = wakeupIn;
    esp_sleep_enable_timer_wakeup(wakeupIn * uS_TO_S_FACTOR);
    LogInfo.log(LOG_VERBOSE, "Setup ESP32 to wake up after %i Seconds", wakeupIn);
}

/**
 * Get how long to wait before waking up from sleep in seconds
 * 
 * @return How many seconds to wait
 */
uint32_t WakeUpInfoClass::getWakeupInterval()
{
    return this->_wakeupIn;
}

/**
 * Set how long to wait before going to sleep in seconds
 * 
 * @param sleepIn How many seconds to wait
 */
void WakeUpInfoClass::setSleepTime(uint32_t sleepIn)
{
    this->_sleepIn = sleepIn;
    LogInfo.log(LOG_VERBOSE, "Setup ESP32 to sleep in %i Seconds", sleepIn);
}

/**
 * Get how long to wait before sleeping in seconds
 * 
 * @return How many seconds to wait
 */
uint32_t WakeUpInfoClass::getSleepTime()
{
    return this->_sleepIn;
}

/**
 * Suspend can sleep test, maybe not correct to sleep at that point in time.
 */
void WakeUpInfoClass::suspendSleep()
{
    this->_flag++;
}

/**
 * Resume can sleep test
 */
void WakeUpInfoClass::resumeSleep()
{
    this->_flag--;

    if (this->_flag < 0)
    {
        this->_flag = 0;
    }
}

/**
 * Did we wakeup because of the power/reset button
 * 
 * @return True if wake up because of power/reset button
 */
boolean WakeUpInfoClass::isPoweredOn()
{
    return this->_isPowerReset;
}

/**
 * Check if it is time to go to sleep or not
 */
void WakeUpInfoClass::tick()
{
    if (this->_flag == 0)
    {
        if (((millis() - this->_last_check) / ms_TO_S_FACTOR) >= this->getSleepTime())
        {
            this->_last_check = millis();
            if (Configuration.shouldSave())
            {
                Configuration.save();
            }
            if (this->_flag == 0)
            {
                LogInfo.log(LOG_INFO, F("Going to sleep now"));
                _bootTime += millis();
                LogInfo.log(LOG_VERBOSE, "Been alive for %lu seconds", _bootTime / 1000);
                esp_deep_sleep_start();
            }
        }
    }
}

WakeUpInfoClass WakeUp;