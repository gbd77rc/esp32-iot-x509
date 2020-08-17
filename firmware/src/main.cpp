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
    // WiFiInfo.begin();
    EnvSensor.begin(xSemaphore);
    LiDARInfo.begin(xSemaphore);
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
    Configuration.add(&LiDARInfo);
    Configuration.add(&GpsSensor);
    Configuration.load();
    LedInfo.switchOn(LED_POWER);
    OledDisplay.clear();
    OledDisplay.displayLine(0,10,"ID : %s", DeviceInfo.getDeviceId());
    OledDisplay.displayLine(0,20,"Loc: %s", DeviceInfo.getLocation());     
    LogInfo.log(LOG_VERBOSE,"Connecting to sensors");
    EnvSensor.connect();
    LiDARInfo.connect();
    LedInfo.blinkOn(LED_WIFI);

    GpsSensor.connect();

    LedInfo.blinkOn(LED_CLOUD);
    // OledDisplay.displayLine(0,10,F("123456789012345678901234567890"));
    // OledDisplay.displayLine(0,20,F("123456789012345678901234567890"));
    // OledDisplay.displayLine(0,30,F("123456789012345678901234567890"));
    // OledDisplay.displayLine(0,40,F("123456789012345678901234567890"));
    // OledDisplay.displayLine(0,50,F("123456789012345678901234567890"));
    // OledDisplay.displayLine(0,60,F("123456789012345678901234567890"));
    delay(2000);
    // OledDisplay.displayLine(0,10,F("Dev : Blink Lights"));
    // OledDisplay.displayLine(0,20,"ID  : %s", DeviceInfo.deviceId());
    // OledDisplay.displayLine(0,30,"Loc : %s", DeviceInfo.location());
    // OledDisplay.displayLine(0,40,"WiFi: %s", "connecting....");
    // WiFiInfo.connect(0,40);
    // NTPInfo.begin();
    // LiDARInfo.connect();
    // GpsInfo.connect();
    // OledDisplay.displayLine(0,60,F("Initialize Completed....."));    
    // delay(1000);
 
    OledDisplay.displayLine(0,30,"Env: %s", EnvSensor.toString());  
    OledDisplay.displayLine(0,40,"Tim: %s", NTPInfo.getFormattedTime()); 
    OledDisplay.displayLine(0,50,"GPS: %s", GpsSensor.toString());      
    // OledDisplay.displayLine(0,30,F("Time: "));
    // OledDisplay.displayLine(0,40,F("Dist: "));
    // OledDisplay.displayLine(0,50,F("GPS : Lat: "));     
    // OledDisplay.displayLine(0,60,F("      Lng: "));     
}

void loop()
{
    // LiDARInfo.resumeTask();
    // GpsInfo.resumeTask();
    OledDisplay.displayLine(30,40,"%s", NTPInfo.getFormattedTime());
    // OledDisplay.displayLine(36,40,"%icm", LiDARInfo.getDistance());
    // OledDisplay.displayLine(64,50,"%09.5f", GpsInfo.getLat());
    // OledDisplay.displayLine(64,60,"%09.5f", GpsInfo.getLong());
    if (millis() > blinkUntil)
    {
        LedInfo.setBrightness(10);
        LedInfo.blinkOff(LED_CLOUD);
    }
    if (millis() > blinkUntil*2)
    {
        LedInfo.blinkOff(LED_WIFI);
    }    
    EnvSensor.tick();
    delayMicroseconds(10000);
    LiDARInfo.tick();        
    GpsSensor.tick();

    //NTPInfo.tick();
    delay(5000);
    DynamicJsonDocument payload(800);
    auto root = payload.to<JsonObject>();
    LedInfo.toJson(root);
    EnvSensor.toJson(root);
    GpsSensor.toJson(root);
    LogInfo.log(LOG_VERBOSE, F("Current State"), root);
    OledDisplay.displayLine(30,30,"%s", EnvSensor.toString());  
    OledDisplay.displayLine(30,50,"%s", GpsSensor.toString());  
    delay(1000);
}