#include "DeviceInfo.h"
#include "LogInfo.h"

void DeviceInfoClass::begin()
{

}

void DeviceInfoClass::load(JsonObjectConst obj)
{
    strcpy(this->_prefix, obj.containsKey("prefix") ?obj["prefix"].as<const char*>() : "DEVTMP");
    strncpy(this->_device_id, this->_prefix, sizeof(this->_prefix));
    strcat(this->_device_id, "-");
    strcat(this->_device_id, LogInfo.getUniqueId());
    strcpy(this->_location, obj.containsKey("location") ? obj["location"].as<const char*>() : "");
    // uint32_t wakeup = obj["wakeup"].as<int>();
    // if (wakeup > 0)
    // {
    //     WakeUp.setTimerWakeUp(wakeup);
    // }
    LogInfo.log(LOG_INFO, "Device Id           : %s", this->deviceId());
    LogInfo.log(LOG_INFO, "Location            : %s", this->location());
}

const char* DeviceInfoClass::deviceId()
{
    return this->_device_id;
}

void DeviceInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    json["prefix"] = this->_prefix;
    // json["wakeup"] = WakeUp.getWakeupInterval();
    json["location"] = this->_location;
}

void DeviceInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject(this->getSectionName());
    json["device_id"] = this->deviceId();
    json["location"] = this->_location;
}

bool DeviceInfoClass::setLocation(const char* newLocation)
{
    if (strcmp(this->_location, newLocation) != 0)
    {
        LogInfo.log(LOG_VERBOSE, "Location has changed! (%s)", newLocation);
        strcpy(this->_location, newLocation);
        this->_changed = true;
        return true;
    }
    return false;
}

const char* DeviceInfoClass::location()
{
    return this->_location;
}

DeviceInfoClass DeviceInfo;