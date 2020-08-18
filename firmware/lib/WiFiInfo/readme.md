# NTP Information

This library contain the WiFi functionality.  It will be a single instance class, as we create it automatically after defining it.  The instance name `WiFiInfo`.

Each function has been commented.

It will automatically switch into WPS mode if it cannot connect to a known SSID or has never connected.

## Example of use

    WiFiInfo.begin();
    WiFiInfo.connect(0,10);

    if (WiFiInfo.getIsConnected())
    {
        // WiFi is connected
    }
