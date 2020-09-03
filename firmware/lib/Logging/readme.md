# LogInfo

This library write the log formatted messages to the standard series port.  It will be a single instance class, as we create it automatically after defining it.  The instance name `LogInfo`.

Each function has been commented.

## Example of use

    LogInfo.begin();
    Configuration.begin("/config.json");
    Configuration.add(&LogInfo);
    Configuration.load();    

    LogInfo.log(LOG_ERROR, F("This is a problem"));
    LogInfo.log(LOG_WARNING, "This is a problem: %s", "This is the error Msg");
    LogInfo.log(LOG_INFO, F("Logging Config") LogInfo.toJson());

