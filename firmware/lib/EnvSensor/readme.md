# Environment Sensor Library

This library will handle the communication between the DHT-22.  It will also count the number of reading between wakeup session if the wakeup is not of manual power on or reset.

Configuration of the instance is done via the `begin` method.

    boolean begin(ScaleType scaleType = ENV_CELSIUS, boolean testMode = false);

The temperature scaling value is set using this enum

    typedef enum
    {
        ENV_CELSIUS = 1,
        ENV_KELVIN = 2,
        ENV_FAHRENHEIT = 3
    } ScaleType;

Test mode basically just returns static values for the temperature, humidity and pressure.

The `toString` method will fill a string buffer with the current read values, e.g.

    Temperature   : 24.2C    
    Humidity      : 40%    
    Pressure      : 99134.65 Pa    
    Reading Count : 4

## Usuage
The use `EnvSensor`, do the following.

    void setup()
    {    
        if (EnvSensor.begin(ENV_CELSIUS) == false)
        {
            M5.Lcd.fillScreen(RED);
            Serial.println(F("Problem with Sensor Class - Restarting in 10 seconds!"));
            delay(10000);
            ESP.restart(); // Force the restart of the device
        }
    }

    void loop()
    {
        EnvSensor.read();
        static char temp[1024];
        EnvSensor.toString(temp);   
        delay(1000); // Sleep for a second
    }

> _**Important**_  
DO NOT USE `EnvSensor.read()` within an ISR function because it uses `delay(50)` function within one of the private functions.    