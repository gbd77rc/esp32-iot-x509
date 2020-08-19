#include <Arduino.h>

#include <SPIFFS.h>

#include "LogInfo.h"
#include "Display.h"
#include "DeviceInfo.h"
#include "WiFiInfo.h"
#include "NTPInfo.h"
#include "GpsInfo.h"
#include "LedInfo.h"
#include "EnvSensor.h"

SemaphoreHandle_t xSemaphore;

long blinkUntil = 10000;

void setup()
{
    Serial.begin(115200);
    xSemaphore = xSemaphoreCreateMutex();

    LogInfo.begin();
    OledDisplay.begin();
    DeviceInfo.begin();
    WiFiInfo.begin();
    EnvSensor.begin(xSemaphore);
    GpsSensor.begin(xSemaphore);
    LedInfo.begin();

    if (!SPIFFS.begin(true))
    {
        OledDisplay.displayExit(F("An Error has occurred while mounting SPIFFS"));
    }

    Configuration.begin("/config.json");
    Configuration.add(&LogInfo);
    Configuration.add(&LedInfo);
    Configuration.add(&DeviceInfo);
    Configuration.add(&EnvSensor);
    Configuration.add(&GpsSensor);
    Configuration.load();
    LedInfo.switchOn(LED_POWER);
    OledDisplay.clear();
    OledDisplay.displayLine(0, 10, "ID : %s", DeviceInfo.getDeviceId());
    OledDisplay.displayLine(0, 20, "Loc: %s", DeviceInfo.getLocation());
    LogInfo.log(LOG_VERBOSE, "Connecting to sensors");
    EnvSensor.connect();
    GpsSensor.connect();
    delay(2000);

    WiFiInfo.connect(0, 30);

    if (WiFiInfo.getIsConnected())
    {
        OledDisplay.displayLine(0, 40, "Tim: %s", NTPInfo.getFormattedTime());
        OledDisplay.displayLine(0, 50, "Env: %s", EnvSensor.toString());
        OledDisplay.displayLine(0, 60, "GPS: %s", GpsSensor.toString());
        LogInfo.log(LOG_VERBOSE, "Startup Completed at %s", NTPInfo.getISO8601Formatted().c_str());
    }
    else
    {
        OledDisplay.displayExit(F("Not Connected to WiFi so rebooting as it pointless continuing!"), 30);
    }
}

void loop()
{
    if (WiFiInfo.getIsConnected())
    {
        OledDisplay.displayLine(30, 50, "%s", NTPInfo.getFormattedTime());
        EnvSensor.tick();
        GpsSensor.tick();
        NTPInfo.tick();
        // DynamicJsonDocument payload(800);
        // auto root = payload.to<JsonObject>();
        // LedInfo.toJson(root);
        // EnvSensor.toJson(root);
        // GpsSensor.toJson(root);
        // LogInfo.log(LOG_VERBOSE, F("Current State"), root);
        WakeUp.tick();
        OledDisplay.displayLine(30, 40, "%s", EnvSensor.toString());
        OledDisplay.displayLine(30, 60, "%s", GpsSensor.toString());
        delay(1000);
    }
}