# WakeUp Information Library
This library will handle the wakeup via timer and sleep to functions and detection around why it was waked up.  It will try to keep the time the device is alive, but will reset this time if the device is manually reset or powered on.

The `RTC_DATA_ATTR` macro is used to store the value within slow memory and therefore will be still available on reboots/resets.  It is necessary to check the type of wakeup to see if these variables need to be reset or not.

The detection of the wake up type is done via the `begin` method.

    void begin();

If a wakeup timer is needed then this configured via the `setTimerWakeUp` method.

    void setTimerWakeUp(unsigned int sleepTime);  // The number of seconds

If a wakeup on pin is needed then this configured via the `setExternalPinWakeUp` method.  There can only be one pin set, so if you assign multiple different pins then the last pin will win.

    void setExternalPinWakeUp(gpio_num_t pin, int level = 0);  // Which pin and if LOW or HIGH level is used.

> _**M5Stack Buttons**_  
Something to note is that pressing a button set the pin to low, which is the reserve of what you may think.

> _**Information**_  
This class could use the `M5.Power` functions to detect power on states, but it is the underlying ESP32 features directly and there can be used with anyt ESP32 board.

    // GPIO Pin Mask 39 38 37 36 35 34 33 32 | 31 => 0
    //                1  1  0  0 0  0  0   0    00000000000000000000000000000000
    //                00100000000000000000000000000000000000000
    // Convert to Hex C000000000
    //uint64_t mask = 0xC000000000;
    //uint64_t mask = 0x2000000;     // GPIO 25
    //uint64_t mask = 0x6000000000;  // GPIO 37 & 38

    //WakeUpInfo.setMultipleExternalPinWakeUp(mask,1);


