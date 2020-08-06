#include "LedInfo.h"
#include "Logging.h"

void LedInfoClass::begin()
{

}

void LedInfoClass::load(JsonObjectConst obj)
{
    this->_isEnabled = obj.containsKey("enabled") ? obj["enabled"].as<bool>() : true;
    this->_pins[LedType::LED_POWER] = obj.containsKey("power") ? obj["power"].as<uint16_t>() : 26;
    this->_pins[LedType::LED_READ] = obj.containsKey("read") ? obj["read"].as<uint16_t>() : 25;
    this->_pins[LedType::LED_WRITE] = obj.containsKey("write") ? obj["write"].as<uint16_t>() : 24;
    initialise();
    Logging.log(LOG_INFO, "Power Pin: %i Read Pin: %i Write Pin: %i",
        this->_pins[LedType::LED_POWER],
        this->_pins[LedType::LED_READ],
        this->_pins[LedType::LED_WRITE]);
}

void LedInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    //json["level"] = (int)this->_reportingLevel;
    json["enabled"] = this->_isEnabled;
    json["power"] = this->_pins[LedType::LED_POWER];
    json["read"] = this->_pins[LedType::LED_READ];
    json["write"] = this->_pins[LedType::LED_WRITE];
}

void LedInfoClass::toJson(JsonObject ob)
{
    //Nothing to show.
}

bool LedInfoClass::setIsEnabled(bool flag)
{
    if(this->_isEnabled != flag)
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