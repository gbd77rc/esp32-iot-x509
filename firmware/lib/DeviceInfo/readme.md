# DisplayInfo

This library contain the device specific properties.  It will be a single instance class, as we create it automatically after defining it.  The instance name `DeviceInfo`.

It has the following properties available.

* Device Id - Will be the combination of the prefix in the config file and the CPU Id
* Location - The user settable property on where the device is located.

Each function has been commented.

## Example of use

    DeviceInfo.begin();
    Configuration.begin("/config.json");
    Configuration.add(&DeviceInfo);
    Configuration.load();    

    LogInfo.log(LOG_VERBOSE, "Device Id is %s", DeviceInfo.getDeviceId());
    LogInfo.log(LOG_VERBOSE, "Location is %s", DeviceInfo.getLocation());

    DeviceInfo.setLocation("Office 1A");
