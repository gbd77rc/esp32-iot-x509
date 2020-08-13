# Firmware Development

I will not be going thought the firmware line by line as that will be boring.  I will leave it up to you to decide you wish to do that.  Instead I will go through the why of each library and what to look out for.  By the time these blogs are published I will have commented each library and give some examples where I think the code is not clear enough.

## Platform IO - Non-standard

If you open up the `platformio.ini` file in the root of the github repo you will notice this sections

    [platformio]
    data_dir = firmware/data
    src_dir = firmware/src
    include_dir = firmware/include
    lib_dir = firmware/lib

Normally, this would not be included.  As I will be including multiple different sources in this repo, generally just to keep them all self contained, I have add it in.  

For general development practices, each app, program, website, documentation would have its own repo to build from.  This will be covered in the [CI/CD Pipelines](./CICDPipeLines.md) blog.

## Multiple Threads/Tasks

The ESP32 device have 2 cores, this allows for multiple processes/task to be performed at once.  The timing is will be issue here with the possible race conditions that could follow, like reading of pins that share the same hardware interrupts. With that being said some of tasks will take Mutex before beginning checking the sensor and then hand back the Mutex when finished.  

Not all tasks will require to be ran on core 0 either.  We will see what the balance is like during the development.  Currently thinking that the check for cloud messages can be ran on the core 1, along with the other standard functions for display and loop functions.

## Libraries

The firmware will use a mix of libraries from 3rd party or ones we will make.  

All 3rd party libraries are registered in the `platformio.ini` file, in the `lib_deps` setting.

All developed libraries are in the `lib` folder.  By the time this published you should find the following in each sub folder.  The `.cpp` and `.h` files that contain the library and a `readme.md` that will explain it.

## Development IDE

The recommended IDE from Platform.io is to use [Visual Studio Code](https://code.visualstudio.com/download) and use the [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) extension.  Personally I have a lot more extensions added to cloud interaction, formatting, docker/containers etc.

This is a great open source free IDE.  It can be a bit limiting sometimes, depends on the language you are using.  

## Language

The firmware is going to be written in C/C++ version 11, so we are not going to be using any of the fancy features that 14 and 17 give.  That being said, we don't really need them.

## ESP32 Development Board

This board has some nice features, like inbuilt WiFi controller, 8MB of flash storage, low power draw when in deep sleep (800uA) to name a few.  The is a different version of the board that support LoRAN communication as well.  If I have time at the end I will try to incorporate that version with this setup.

## Lets Get Started

All sensors, logging and sleep/wake are controlled via a configuration.  So to make this easier to manipulate we will create a base class that this libraries can inherit from.  Therefore the first library to be created is the configuration manipulation one.

### Configuration

This library will have two classes defined in it. The `BaseConfigInfoClass` is really just an interface that each class the needs to handle configuration must inherit from.  The `ConfigClass` handles the actual loading/saving of the configuration file and the logic around when to save.  We don't want to save always, as their is limit on the number times the flash memory can be written to.

The `ConfigClass` does make uses of the `LogInfoClass`, so you may think there is race/deadlock condition here, as logging requires configuration.  It does not, as it just sets the level that the log messages are sent to the serial port and nothing else.

#### Arduino JSON

The configuration library depends heavily on the [ArduinoJSON](https://arduinojson.org/) v6 library for handling of the JSON file.  I would recommended at least reading their [common errors and problems](https://arduinojson.org/v6/error/) section.  The single instance name is `Configuration`.

### Logging

The logging library uses the configuration to determine which level it will report on.  The single instance name is `LogInfo`.  Why LogInfo, basically because this is another 3rd party library that uses `Logging` as their instance name, so to avoid conflicts I renamed mine.

### LED Control

There are 3 LED's to control.  These will give out state to the local environment, if someone is watching that it.  The following states are.

|Colour|State|Meaning
|---|---|---
|Power|Blinking|Initialising System
|Power|Steady|Working Normally
|Power|Off|Deep Sleep Mode
|WiFi|Blinking|Try to connect to WiFi
|WiFi|Steady|Connect to WiFi and have access to internet
|WiFi|Off|If Power On then WiFi off means no internet detected or can't connect to router
|Cloud|Blinking|Send/Receive Data
|Cloud|Steady|Connected to the cloud provider successfully
|Cloud|Off|If Power On and WiFi On then issue with cloud provider communication

The blinking state will be handle by a separate task. This task will check a state flag, and if set will automatically set the LED on/off depending on the state flag and then delete itself.

To disable LED's set the brightness level to 0;

This library relies on the [ESP32 analogWrite](https://github.com/ERROPiX/ESP32_AnalogWrite) library and the native framework for ESP32 does not support support Arduino analogWrite function.

> Interesting this library uses [Task Notification](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#_CPPv411xTaskNotify12TaskHandle_t8uint32_t13eNotifyAction) message to control blinking on or off.  

The single instance name is `LedInfo`.

### Device Information

This library is a catch all one, in that it contains the actual full ID, location and wake up details at least.  During the development if other options that are specific to the device they may appear here.

### Environment Sensor

This library will handle environment 