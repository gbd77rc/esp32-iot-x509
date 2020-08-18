# GPS Sensor Library

This library will handle the communication between the NEO-6MP GPS sensor.  It will also count the number of reading between wakeup session if the wakeup is not of manual power on or reset.  It will be a single instance class, as we create it automatically after defining it.  The instance name `GpsSensor`.

Configuration of the instance is done via the `begin` method.

    boolean begin(SemaphoreHandle_t flag);

The reading of the GPS is via UART (TX,RX) serial communication.

The `toJson` method will fill a json element with the current read values, e.g.

    "GPS": {
        "location": {
        "longitude": 0.484414,
        "latitude": 51.74432,
        "satellites": 0,
        "course": 2852,
        "speed": 2,
        "altitude": 0,
        "timestamp": 960056
        }
    }

You notice that the satellites, altitude values are 0.  This is really dependent on the satellite fix and if we have read the NMEA sentences being read.  This is is part of the TinyGPSPlus library.

The `tick` function must be called regularly.

## Usage

The use `GpsSensor`, do the following.

    SemaphoreHandle_t xSemaphore = xSemaphoreCreateMutex();
    GpsSensor.begin(xSemaphore);
    Configuration.begin("/config.json");
    Configuration.add(&GpsSensor);    
    Configuration.load();
    GpsSensor.connect();

    GpsSensor.tick();
