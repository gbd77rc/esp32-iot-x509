#include <Arduino.h>

#include "SPIFFS.h"

#include "LogInfo.h"
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

    LogInfo.begin();
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
    Configuration.add(&LogInfo);
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
    OledDisplay.displayLine(0,60,F("Initialize Completed....."));    
    delay(1000);
    OledDisplay.clear();
    OledDisplay.displayLine(0,10,"ID  : %s", DeviceInfo.deviceId());
    OledDisplay.displayLine(0,20,"Loc : %s", DeviceInfo.location());  
    OledDisplay.displayLine(0,30,F("Time: "));
    OledDisplay.displayLine(0,40,F("Dist: "));
    OledDisplay.displayLine(0,50,F("GPS : Lat: "));     
    OledDisplay.displayLine(0,60,F("      Lng: "));     
}

void loop()
{
    LiDARInfo.resumeTask();
    GpsInfo.resumeTask();
    OledDisplay.displayLine(36,30,"%s", NTPInfo.getFormattedTime());
    OledDisplay.displayLine(36,40,"%icm", LiDARInfo.getDistance());
    OledDisplay.displayLine(64,50,"%09.5f", GpsInfo.getLat());
    OledDisplay.displayLine(64,60,"%09.5f", GpsInfo.getLong());
    delay(500);
}