#include <Arduino.h>

#include "SPIFFS.h"

#include "Logging.h"
#include "Display.h"
#include "DeviceInfo.h"
#include "WiFiInfo.h"
#include "NTPInfo.h"
#include "LiDARInfo.h"

const int redPin = 27;
const int greenPin = 26;
SemaphoreHandle_t xSemaphore;

void setup()
{
    Serial.begin(115200);
    xSemaphore = xSemaphoreCreateMutex();
    LiDARInfoClass::semaphoreFlag = xSemaphore;

    Logging.begin();
    OledDisplay.begin();
    DeviceInfo.begin();
    WiFiInfo.begin();
    LiDARInfo.begin();

    if (!SPIFFS.begin(true))
    {
        OledDisplay.displayExit(F("An Error has occurred while mounting SPIFFS"));
    }    

    Configuration.begin("/config.json");
    Configuration.add(&Logging);
    Configuration.add(&DeviceInfo);
    Configuration.add(&LiDARInfo);
    Configuration.load();

    OledDisplay.displayLine(0,10,F("Dev : Blink Lights"));
    OledDisplay.displayLine(0,20,"ID  : %s", DeviceInfo.deviceId());
    OledDisplay.displayLine(0,30,"Loc : %s", DeviceInfo.location());
    OledDisplay.displayLine(0,40,"WiFi: %s", "connecting....");
    WiFiInfo.connect(0,40);
    NTPInfo.begin();
    LiDARInfo.connect();

    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
}

void loop()
{
    NTPInfo.tick();
    OledDisplay.displayLine(0, 50, "Time: %s  ", NTPInfo.getFormattedTime().c_str());
    OledDisplay.displayLine(0, 60, "Led : %s  ", "ON");
    digitalWrite(redPin, HIGH);
    delay(1000);
    digitalWrite(greenPin, HIGH);
    delay(1000);
    digitalWrite(greenPin, LOW);
    delay(500); §
    digitalWrite(redPin, LOW);
    OledDisplay.displayLine(0, 60, "Led : %s  ", "OFF");
    delay(1000);
}