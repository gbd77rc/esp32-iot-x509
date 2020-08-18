# Environment Sensor Library

This library will handle the communication between the DHT-22.  It will also count the number of reading between wakeup session if the wakeup is not of manual power on or reset.  It will be a single instance class, as we create it automatically after defining it.  The instance name `EnvSensor`.

Configuration of the instance is done via the `begin` method.

    boolean begin(SemaphoreHandle_t flag);

The temperature scaling value is set using this enum

    typedef enum
    {
        ENV_CELSIUS = 1,
        ENV_KELVIN = 2,
        ENV_FAHRENHEIT = 3
    } ScaleType;

Test mode basically just returns static values for the temperature and humidity..

The `toJson` method will fill a json element with the current read values, e.g.

    "EnvSensor": {
        "temperature": 25,
        "humidity": 66.3,
        "read_count": 32,
        "epoch": 1597598255
    }

The `tick` function must be called regularly.

## Usage

The use `EnvSensor`, do the following.

    SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();
    EnvSensor.begin(xSemaphore);
    Configuration.begin("/config.json");
    Configuration.add(&EnvSensor);    
    Configuration.load();
    EnvSensor.connect();

    EnvSensor.tick();
