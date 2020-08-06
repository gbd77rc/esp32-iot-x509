#include <Arduino.h>

#include "SPIFFS.h"

#include "Logging.h"
#include "Display.h"
#include "DeviceInfo.h"
#include "WiFiInfo.h"
#include "NTPInfo.h"
#include "LiDARInfo.h"
#include "GpsInfo.h"
#include "LedInfo.h"

SemaphoreHandle_t xSemaphore;

void setup()
{
    Serial.begin(115200);
    xSemaphore = xSemaphoreCreateMutex();
    LiDARInfoClass::semaphoreFlag = xSemaphore;
    GpsInfoClass::semaphoreFlag = xSemaphore;

    Logging.begin();
    OledDisplay.begin();
    DeviceInfo.begin();
    WiFiInfo.begin();
    LiDARInfo.begin();
    GpsInfo.begin();
    LedInfo.begin();

    if (!SPIFFS.begin(true))
    {
        OledDisplay.displayExit(F("An Error has occurred while mounting SPIFFS"));
    }    

    Configuration.begin("/config.json");
    Configuration.add(&Logging);
    Configuration.add(&LedInfo);
    Configuration.add(&DeviceInfo);
    Configuration.add(&LiDARInfo);
    Configuration.add(&GpsInfo);
    Configuration.load();
    LedInfo.switchOn(LED_POWER);
    OledDisplay.displayLine(0,10,F("Dev : Blink Lights"));
    OledDisplay.displayLine(0,20,"ID  : %s", DeviceInfo.deviceId());
    OledDisplay.displayLine(0,30,"Loc : %s", DeviceInfo.location());
    OledDisplay.displayLine(0,40,"WiFi: %s", "connecting....");
    WiFiInfo.connect(0,40);
    NTPInfo.begin();
    LiDARInfo.connect();
    GpsInfo.connect();
}

void loop()
{
    NTPInfo.tick();
    OledDisplay.displayLine(0, 50, "Time: %s  ", NTPInfo.getFormattedTime().c_str());
    OledDisplay.displayLine(0, 60, "Led : %s  ", "ON");
    LedInfo.switchOn(LED_READ);
    delay(1000);
    LedInfo.switchOn(LED_WRITE);
    delay(1000);
    LedInfo.switchOff(LED_WRITE);
    delay(500);
    LedInfo.switchOff(LED_READ);
    delay(500);
    OledDisplay.displayLine(0, 60, "Led : %s  ", "OFF");    
    GpsInfo.resumeTask();
    delay(1000);
}