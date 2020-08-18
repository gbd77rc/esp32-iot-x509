# Led Information

This library contain the device specific properties.  It will be a single instance class, as we create it automatically after defining it.  The instance name `LedInfo`.

Each function has been commented.

## Example of use

    LedInfo.begin();
    Configuration.begin("/config.json");
    Configuration.add(&LedInfo);
    Configuration.load();    

    LedInfo.switchOn(LED_POWER);

    LedInfo.blinkOn(LED_WIFI);

    delay(10000);
    LedInfo.blinkOff(LED_WIFI);
