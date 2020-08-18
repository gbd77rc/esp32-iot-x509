# NTP Information

This library contain the NTP specific properties.  It will be a single instance class, as we create it automatically after defining it.  The instance name `NTPInfo`.

Each function has been commented.

The `tick` function must be called regularly.

## Example of use

    NTPInfo.begin();
    Configuration.begin("/config.json");
    Configuration.add(&NTPInfo);
    Configuration.load();    

    Serial.printf("Current Date/Time is %s\n", NTPInfo.getISO8601Formatted());

    long timeStamp = NTPInfo.getEpoch();

    NTPInfo.tick();
