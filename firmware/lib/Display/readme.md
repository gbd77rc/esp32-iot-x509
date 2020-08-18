# Display

This library write the formatted messages to the standard to the embedded OLED screen.  It will be a single instance class, as we create it automatically after defining it.  The instance name `OledDisplay`.

The OLED display has a resolutions of 128 * 64 pixels.  Depending on the font/font size (8pt being used here), this could mean you have a display of 22 characters by 6 lines of text.  I leave it up to the user if they need to change the font, font size etc.  You the use the [U8g2 Wiki](https://github.com/olikraus/u8g2/wiki) for help.

> Side note - You must run `OledDisplay.begin()` before trying to write anything to the screen or else it will reset itself with a panic exception.

Each function has been commented.

## Example of use

    OledDisplay.begin();
    OledDisplay.displayLine(0,10,F("Dev : Blink Lights"));
    OledDisplay.displayLine(0,20,"ID  : %s", DeviceInfo.deviceId()); 
    // Use in the setup function as this is static text
    OledDisplay.displayLine(0,30,F("Time: "));
    OledDisplay.displayLine(0,40,F("Dist: "));
    OledDisplay.displayLine(0,50,F("GPS : Lat: "));     
    
    
    // Run in the loop function as this text change change.  It will reduce the amount of flicker the eye sees
    OledDisplay.displayLine(36,30,"%s", NTPInfo.getFormattedTime());
    OledDisplay.displayLine(36,40,"%icm", LiDARInfo.getDistance());
    OledDisplay.displayLine(64,50,"%09.5f", GpsInfo.getLat());
