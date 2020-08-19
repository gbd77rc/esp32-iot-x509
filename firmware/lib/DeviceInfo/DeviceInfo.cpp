#include "DeviceInfo.h"
#include "LogInfo.h"
#include "WakeUpInfo.h"


/**
 * Initialise the device and wakeup
 */
void DeviceInfoClass::begin()
{
    WakeUp.begin();
}

/**
 * overridden load JSON element into the device instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void DeviceInfoClass::load(JsonObjectConst obj)
{
    strcpy(this->_prefix, obj.containsKey("prefix") ?obj["prefix"].as<const char*>() : "DEVTMP");
    strncpy(this->_device_id, this->_prefix, sizeof(this->_prefix));
    strcat(this->_device_id, "-");
    strcat(this->_device_id, LogInfo.getUniqueId());
    strcpy(this->_location, obj.containsKey("location") ? obj["location"].as<const char*>() : "");
    uint32_t wakeup = obj["wakeup"].as<int>();
    WakeUp.setSleepTime(obj.containsKey("sleep") ? obj["sleep"].as<int>() : 30);   
    if (wakeup > 0)
    {
        WakeUp.setTimerWakeUp(wakeup);
    }
    LogInfo.log(LOG_INFO, "Device Id           : %s", this->getDeviceId());
    LogInfo.log(LOG_INFO, "Location            : %s", this->getLocation());
}

/**
 * overridden save JSON element from the device instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void DeviceInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    json["prefix"] = this->_prefix;
    json["wakeup"] = WakeUp.getWakeupInterval();
    json["sleep"] = WakeUp.getSleepTime();
    json["location"] = this->_location;
}

/**
 * overridden create a JSON element that will show the current device related instance data
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */
void DeviceInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject(this->getSectionName());
    json["device_id"] = this->getDeviceId();
    json["location"] = this->getLocation();
}

/**
 * overridden save JSON element from the led instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
const char* DeviceInfoClass::getDeviceId()
{
    return this->_device_id;
}

/**
 * set the current location
 * 
 * @param newLocation The new location name
 * @return True if it has changed
 */
bool DeviceInfoClass::setLocation(const char* newLocation)
{
    if (strcmp(this->_location, newLocation) != 0)
    {
        strcpy(this->_location, newLocation);
        this->_changed = true;
        return true;
    }
    return false;
}

/**
 * Get the current location
 * 
 * @return Pointer to the location name string
 */
const char* DeviceInfoClass::getLocation()
{
    return this->_location;
}

DeviceInfoClass DeviceInfo;