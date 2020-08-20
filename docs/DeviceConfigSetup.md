# Device Configuration Setup

The following information describes the configuration file layout and the sensor related information.

## Configuration File Layout

    {
        "logging": {
            "level": "ALL"
        },
        "ledInfo": {
            "brightness": 100,
            "sensors": 25,
            "cloud": 26,
            "power": 27
        },
        "gpsSensor": {
            "enabled": true,
            "tx": 23,
            "rx": 22,
            "baud": 9600,
            "sampleRate": 1000
        },
        "envSensor": {
            "enabled": true,
            "data": 14,
            "scale": 1,
            "sampleRate": 20000
        },
        "device": {
            "prefix": "OT",
            "wakeup": 1200,
            "sleep": 30,
            "location": "<UNKNOWN>"
        },
        "certs":{
            "certificate": "/cloud/cert.pem",
            "key": "/cloud/key.pem"
        },
        "iotHub":{
            "endpoint": "",
            "name": "",
            "port": 8883,
            "sendTelemetry": true,
            "intervalSeconds": 60
        },
        "azure": {
            "ca": "/cloud/portal-azure-com.pem"
        },
        "aws":{
            "ca": "/cloud/console-aws-com.pem"
        }
    }

## Logging Section

    "logging": {
        "level": "ALL"
    }

Internally the device will echo various log messages to the Serial Port 1.  The level that will be logged to this port are:

0. Off
1. Error
2. Warning
3. Information
4. Verbose
5. All

Using the level will determine which message types are echoed.  Example if the level is set to Warning, then only Error and Warning messages will be sent, but set to All, then all messages will be sent.

## LED Information Section

    "ledInfo": {
        "brightness": 100,
        "wifi": 25,
        "cloud": 26,
        "power": 27
    }

The LED's will show the user what the device is doing or is active.  The `brightness` is a percentage value.  0 is off and 100 is fully bright.

The configuration is for the pin numbers that control which LED's is for which function.  

The `brightness` flag can set via the cloud Shadow/Device Twin setting.  It will automatically dim the LED's when set.  The pin values, once set should never need changing.

## GPS Information Section

    "gpsInfo": {
        "enabled": true,
        "tx": 23,
        "rx": 22,
        "baud": 9600,
        "sim808": false
    }

The sensor will retrieve the latitude/longitude, altitude, direction, speed and how many satellites are locked data.

The configuration is for the pin numbers used to communicate with the GPS sensor.  The `RX` and `TX` are the pins on the ESP32 device side, not the sensor side.  As with all [UART Protocol](https://www.circuitbasics.com/basics-uart-communication/#:~:text=UART%20stands%20for%20Universal%20Asynchronous,transmit%20and%20receive%20serial%20data.) devices the `RX` and `TX` get crossed over. See the documentation related to UART.

The `baud` rate is how fast the serial communication can happen in bits per seconds.  Most GPS's only transmit at `9600` baud by default.  Some of them you change this, the NEO-6M can be changed, but I have left it at the default.  Not setting able via the cloud.

The `sim808` is another type of GPS, not used in this project.  It relies on GSM/GPRS signals and SIM card.  Not setting able via the cloud.

The `enabled` flag can set via the cloud Shadow/Device Twin setting.  It will automatically switch the sensor on or off when set.  The pin values, once set should never need changing.

## Temperature/Humidity Information Section

    "envSensor": {
        "enabled": true,
        "data": 32,
        "scale": 1,
        "sampleRate": 10000
    }

The sensor will retrieve the current temperature and humidity.  The sensor being used for this is the DHT-22, which has a bigger range and is more sensitive.  

> Temperature Range: 0 to 125 °C +/- 5°C
> Humidity Range: 0 - 100% +/- 2.5%

The `data` pin is used for communication.  The device will use this pin for receiving of data.  Once set should never need changing.

The `enabled` flag can set via the cloud Shadow/Device Twin setting.  It will automatically switch the sensor on or off when set.

The `sampleRate` is how often should it be read.  There is a minimum sample rate of 0.5Hz for DHT22 sensor (about 2.5 seconds).

## Device Information Section

    "device": {
        "prefix": "OT",
        "wakeup": 1200,
        "sleep": 30,
        "location": "<UNKNOWN>"
    }

This is the ESP32 device related information.  

The `prefix` is used in front of the CPU ID, to give it a more identifiable ID. Its short here so that it will fit on the OLED display, but can be any length.  Not settable via the cloud, as it will make up the device ID which is `common name` in the x509 certificate on the device.  The cloud provides check that the CN is the same as the device ID when connecting.

The `wakeup` is used for how often the device should resume from deep sleep.  This can be settable via the cloud.  It may take time to set tho, as it has to wake up first before accepting the new value.

The `sleep` is used for how long to wake before going to sleep.  Set it to zero to never go to sleep.  This can be settable via the cloud.  It may take time to set tho, as it has to wake up first before accepting the new value.

The `location` is the used for where the device is.  Normally which room it is located in, i.e. Home Office.

## Certificate Information Section

    "certs":{
        "certificate": "/cloud/cert.pem",
        "key": "/cloud/key.pem"
    }

These are the certificate and key that identifies the device.

The `certificate` is the file that contains the `CN` of the device.

The `key` is the private key that will encrypt the communication between the device and the cloud provider.  The cloud provider will have the public key to decrypt this information.

The `CA` in the cloud specific sections contains the public key so that the device can decrypt the information being sent from the cloud provider.

## IoT Hub Information Section

    "iotHub":{
        "endpoint": ""
        "name": "",
        "port": 8883,
        "sendTelemetry": true,
        "intervalSeconds": 60
    }

This is the general information that Azure and AWS share for communicating between the IoT Hub broker and device.  Generally it is an MQTT broker under the hoods.  

The `endpoint` is the URL where the IoT Hub/Core is located in the cloud provider.  Not settable via the cloud.

The `name` is the name of the IoT Hub/Core. Not settable via the cloud.

The `port` is the port that the IoT Hub/Core is listening on.

The `sendTelemetry` is the flag to indicate that telemetry data should be sent.  This can be settable via the cloud.  Shadow/Device Twin is alway enabled so that configuration and limited telemetry is sent.

The `intervalSeconds` is the number of seconds to wait before sending telemetry data again, `sendTelemetry` is enabled.  This can be settable via the cloud.

## Azure Information Section

    "azure": {
        "ca": "/cloud/portal-azure-com.pem"
    }

This is the public key that the cloud provider gives out for decryption of information from them.

## AWS Information Section

    "aws":{
        "ca": "/cloud/console-aws-com.pem"
    }

This is the public key that the cloud provider gives out for decryption of information from them.
