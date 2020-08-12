#include "LedInfo.h"
#include "LogInfo.h"

void LedInfoClass::begin()
{

}

void LedInfoClass::load(JsonObjectConst obj)
{
    this->_isEnabled = obj.containsKey("enabled") ? obj["enabled"].as<bool>() : true;
    this->_pins[LedType::LED_POWER] = obj.containsKey("power") ? obj["power"].as<uint16_t>() : 26;
    this->_pins[LedType::LED_LIDAR] = obj.containsKey("lidar") ? obj["lidar"].as<uint16_t>() : 24;
    this->_pins[LedType::LED_GPS] = obj.containsKey("gps") ? obj["gps"].as<uint16_t>() : 25;
    initialise();
    LogInfo.log(LOG_VERBOSE, "Power Pin: %i GPS Pin: %i LiDAR Pin: %i",
        this->_pins[LedType::LED_POWER],
        this->_pins[LedType::LED_GPS],
        this->_pins[LedType::LED_LIDAR]);
}

void LedInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    //json["level"] = (int)this->_reportingLevel;
    json["enabled"] = this->_isEnabled;
    json["power"] = this->_pins[LedType::LED_POWER];
    json["gps"] = this->_pins[LedType::LED_GPS];
    json["lidar"] = this->_pins[LedType::LED_LIDAR];
}

void LedInfoClass::toJson(JsonObject ob)
{
    //Nothing to show.
}

bool LedInfoClass::setIsEnabled(bool flag)
{
    if (this->_isEnabled != flag)
    {
        this->_isEnabled = flag;
        this->_changed = true;
    }
}

void LedInfoClass::initialise()
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        pinMode(this->_pins[i], OUTPUT);
        digitalWrite(this->_pins[i], LOW);
    }
}

void LedInfoClass::switchOn(LedType type)
{
    if (this->_isEnabled)
    {
        digitalWrite(this->_pins[type], HIGH);
    }
}

void LedInfoClass::switchOff(LedType type)
{
    if (this->_isEnabled)
    {
        digitalWrite(this->_pins[type], LOW);
    }
}

LedInfoClass LedInfo;