#include <analogWrite.h>
#include "LedInfo.h"
#include "LogInfo.h"

TaskHandle_t LedInfoClass::blinkTaskHandles[LED_COUNT];

/**
 * LED Blink Task, it will take in a LedState structure to switch the LED ON/OFF every 500ms or near there.  It will 
 * continue to blink until a Task Notification has sent a value greater then 0 to the ulStop variable.
 * 
 * @param parameters The ArduinoJson object that this element will be loaded from
 */
void LedInfoClass::blinkTask(void *parameters)
{
    LedState *pLed = (struct ledStateStruct *)parameters;
    LogInfo.log(LOG_VERBOSE, "Starting to blink for %s on pin %i", pLed->typeName, pLed->pin);
    uint32_t ulStop;
    for (;;)
    {
        xTaskNotifyWait(0x00, ULONG_MAX, &ulStop, 500 / portTICK_PERIOD_MS);
        if (ulStop > 0)
        {
            analogWrite(pLed->pin, 0);
            if (pLed->isOn)
            {
                // Switch back on if we had use switchOn method before the blink
                vTaskDelay(500 / portTICK_PERIOD_MS);
                analogWrite(pLed->pin, 100);
            }
            LogInfo.log(LOG_VERBOSE, "Stopping blinking for %s on pin %i at %i brightness and state is %s", pLed->typeName, pLed->pin, pLed->brightness, pLed->isOn ? "ON" : "OFF");
            vTaskDelete(LedInfoClass::blinkTaskHandles[pLed->idx]);
            LedInfoClass::blinkTaskHandles[pLed->idx] = NULL;
            break; // should not be needed, but just incase.
        }
        analogWrite(pLed->pin, pLed->brightness);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        analogWrite(pLed->pin, 0);
    }
}

/**
 * overridden load JSON element into the led instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void LedInfoClass::load(JsonObjectConst obj)
{
    this->_brightness = obj.containsKey("brightness") ? obj["brightness"].as<uint8_t>() : 100;
    this->_led[LedType::LED_POWER].pin = obj.containsKey("power") ? obj["power"].as<uint8_t>() : 26;
    this->_led[LedType::LED_WIFI].pin = obj.containsKey("wifi") ? obj["wifi"].as<uint8_t>() : 24;
    this->_led[LedType::LED_CLOUD].pin = obj.containsKey("cloud") ? obj["cloud"].as<uint8_t>() : 25;
    initialise();
    LogInfo.log(LOG_VERBOSE, "Power Pin: %i WiFi Pin: %i Cloud Pin: %i Brightness: %i",
                this->_led[LedType::LED_POWER].pin,
                this->_led[LedType::LED_WIFI].pin,
                this->_led[LedType::LED_CLOUD].pin,
                this->_brightness);
}

/**
 * overridden save JSON element from the led instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void LedInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->getSectionName());
    json["brightness"] = this->_brightness;
    json["power"] = this->_led[LedType::LED_POWER].pin;
    json["wifi"] = this->_led[LedType::LED_WIFI].pin;
    json["cloud"] = this->_led[LedType::LED_CLOUD].pin;
}

/**
 * overridden create a JSON element that will show the current brightness level
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */
void LedInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject(this->getSectionName());
    json["brightness"] = this->_brightness;
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        json[this->_led[i].typeName] = this->_led[i].isOn ? "ON" : "OFF";
    }
}

/**
 * set the current brightness level.
 * 
 * @param brightness The new brightness level to set
 */
void LedInfoClass::setBrightness(uint8_t brightness)
{
    if (this->_brightness != brightness)
    {
        this->_brightness = brightness;
        this->_changed = true;
        for (uint8_t i = 0; i < LED_COUNT; i++)
        {
            this->_led[i].brightness = this->_brightness;
            if (_led[i].isOn)
            {
                analogWrite(this->_led[i].pin, this->_brightness);
            }
        }
    }
}

/**
 * initializing the LED setup.  This can't be called begin method as we don't know the pin then.
 */
void LedInfoClass::initialise()
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        strcpy(this->_led[i].typeName, LedInfoClass::ledTypeToString((LedType)i));
        this->_led[i].idx = i;
        pinMode(this->_led[i].pin, OUTPUT);
        analogWrite(this->_led[i].pin, 0);
        this->_led[i].brightness = this->_brightness;
    }
}

/**
 * Switch the specific led type on.
 * 
 * @param type The led brightness to switch on
 */
void LedInfoClass::switchOn(LedType type)
{
    if (this->_brightness > 0)
    {
        LogInfo.log(LOG_VERBOSE, "Switching on %s", this->_led[type].typeName);
        analogWrite(this->_led[type].pin, this->_led[type].brightness);
        this->_led[type].isOn = true;
    }
}

/**
 * Switch the specific led type off.
 * 
 * @param type The led brightness to switch on
 */
void LedInfoClass::switchOff(LedType type)
{
    if (this->_brightness > 0)
    {
        LogInfo.log(LOG_VERBOSE, "Switching off %s", this->_led[type].typeName);
        analogWrite(this->_led[type].pin, 0);
        this->_led[type].isOn = false;
    }
}

/**
 * Blink the LED every 500ms until told to stop.
 * 
 * @param type The led brightness to switch on
 */
void LedInfoClass::blinkOn(LedType type)
{
    if (LedInfoClass::blinkTaskHandles[type] == NULL)
    {
        xTaskCreate(LedInfoClass::blinkTask,
                    "ledBlinking",
                    10000,
                    (void *)&this->_led[type],
                    1,
                    &LedInfoClass::blinkTaskHandles[type]);
    }
}

/**
 * Stop the blinking
 * 
 * @param type The led brightness to switch on
 */
void LedInfoClass::blinkOff(LedType type)
{
    if (LedInfoClass::blinkTaskHandles[type] != NULL)
    {
        xTaskNotify(LedInfoClass::blinkTaskHandles[type], 1, eSetValueWithOverwrite);
    }
}

/**
 * Workout the string for the ledType enum
 * 
 *  @param level The LedType enum level to work with
 *  @return the pointer to the string.
 */
const char *LedInfoClass::ledTypeToString(LedType level)
{
    switch (level)
    {
    case LED_POWER:
        return "power";
    case LED_CLOUD:
        return "cloud";
    case LED_WIFI:
        return "wifi";
    default:
        return "UNK";
    }
    return "UNK";
}

LedInfoClass LedInfo;