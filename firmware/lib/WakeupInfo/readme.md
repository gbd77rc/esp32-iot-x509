# WakeUp Information Library

This library will handle the wakeup via timer and sleep to functions and detection around why it was waked up.  It will try to keep the time the device is alive, but will reset this time if the device is manually reset or powered on.

The `RTC_DATA_ATTR` macro is used to store the value within slow memory and therefore will be still available on reboots/resets.  It is necessary to check the type of wakeup to see if these variables need to be reset or not.

The detection of the wake up type is done via the `begin` method.

    void begin();

If a wakeup timer is needed then this configured via the `setTimerWakeUp` method.

    void setTimerWakeUp(unsigned int sleepTime);  // The number of seconds

The `tick` function will check if it is time to go to sleep or not.

This library will use the `DeviceInfo` library to initialize it.

## Example of Use

    DeviceInfo.begin();
    Configuration.begin("/config.json");
    Configuration.add(&DeviceInfo);
    Configuration.load();
    LogInfo.log(LOG_VERBOSE, "Was Power Button pushed: %s", WakeUp.isPoweredOn());
    WakeUp.tick();
