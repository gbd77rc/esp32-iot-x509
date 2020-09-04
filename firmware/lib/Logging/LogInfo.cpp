#include "LogInfo.h"

/**
 * Begin the initialization of the logging system
 */
void LogInfoClass::begin()
{
    uint64_t chipid;

    chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
                                //It should be unique per ESP32 
    snprintf(this->_uniqueId, 23, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
}

/**
 * overridden load JSON element into the logging instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */ 
void LogInfoClass::load(JsonObjectConst obj)
{
    this->setLogLevel(obj["level"].as<const char*>());
    this->log(LOG_VERBOSE, "Chip Id: %s", this->getUniqueId());
    this->log(LOG_OFF, "Log Level: %s", this->getLogLevel());
    this->_changed = false;
}

/**
 * overridden save JSON element from the logging instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */ 
void LogInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    json["level"] = logTypeToString(this->_reportingLevel);
}

/**
 * overridden create a JSON element that will show the current logging level
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */   
void LogInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject(this->getSectionName());
    json["level"] = logTypeToString(this->_reportingLevel);
}

/**
 * Retrieve the current uniqueId that has been detected.
 * 
 *  @return the pointer to the string.
 */ 
const char* LogInfoClass::getUniqueId()
{
    return this->_uniqueId;
}

/**
 * Workout the short string name for the logType enum
 * 
 *  @param level The LogType enum level to work with
 *  @return the pointer to the string.
 */ 
const char* LogInfoClass::logTypeToShortString(LogType level)
{
    switch (level)
    {
    case LOG_OFF:
        return "OFF";
    case LOG_ERROR:
        return "ERR";
    case LOG_WARNING:
        return "WRN";
    case LOG_INFO:
        return "INF";
    case LOG_VERBOSE:
        return "VRB";
    case LOG_ALL:
        return "ALL";
    default:
        return "UNK";
    }
    return "UNK";
}

/**
 * Workout the string for the logType enum
 * 
 *  @param level The LogType enum level to work with
 *  @return the pointer to the string.
 */ 
const char* LogInfoClass::logTypeToString(LogType level)
{
    switch (level)
    {
    case LOG_OFF:
        return "OFF";
    case LOG_ERROR:
        return "ERROR";
    case LOG_WARNING:
        return "WARNING";
    case LOG_INFO:
        return "INFO";
    case LOG_VERBOSE:
        return "VERBOSE";
    case LOG_ALL:
        return "ALL";
    default:
        return "UNK";
    }
    return "UNK";
}

/**
 * Workout the logType enum value from the string specificed
 * 
 *  @param level The string level to work with
 *  @return the LogType associated with the string
 */ 
LogType LogInfoClass::stringToLogType(const char* level)
{
    if (strcasecmp(level, "OFF") == 0)
    {
        return LOG_OFF;
    }
    if (strcasecmp(level, "ERROR") == 0)
    {
        return LOG_ERROR;
    }
    if (strcasecmp(level, "WARNING") == 0)
    {
        return LOG_WARNING;
    }
    if (strcasecmp(level, "INFO") == 0)
    {
        return LOG_INFO;
    }
    if (strcasecmp(level, "VERBOSE") == 0)
    {
        return LOG_VERBOSE;
    }
    if (strcasecmp(level, "ALL") == 0)
    {
        return LOG_ALL;
    }
    return LOG_WARNING;
}

/**
 * Set the current log report level.
 * 
 *  @param level The logType level to report on.
 */ 
void LogInfoClass::setLogLevel(LogType level)
{
    this->_reportingLevel = level;
}

/**
 * Set the current log report level.
 * 
 *  @param level The logType level to report on.
 */ 
void LogInfoClass::setLogLevel(const char* logType)
{
    this->_reportingLevel = LogInfoClass::stringToLogType(logType);
}

/**
 * Set the current log report level.
 * 
 *  @return The current logType level being reported on
 */ 
const char* LogInfoClass::getLogLevel()
{
    return LogInfoClass::logTypeToString(this->_reportingLevel);
}

/**
 * Write the message to the log level.  The message is using the embedded flash support.
 * 
 *  @param level The logType level being assigned to.
 *  @param ifsh The pointer to the Flash String Helper
 *  @return The size of the string written.
 */ 
size_t LogInfoClass::log(LogType level, const __FlashStringHelper *ifsh)
{
    return this->write(level, reinterpret_cast<const char *>(ifsh));
}

/**
 * Write the message to the log level.  The message is using the embedded flash support.
 * 
 *  @param level The logType level being assigned to.
 *  @param ifsh The pointer to the Flash String Helper
 *  @param object The Arduino JSON object/element to write out
 *  @return The size of the string written.
 */ 
size_t LogInfoClass::log(LogType level, const __FlashStringHelper *ifsh, JsonObject object)
{
    if (level > this->_reportingLevel)
    {
        return 0;
    }
    int len = 0;
    len += writePrefix(level);
    int section = buildSectionHeader(reinterpret_cast<const char *>(ifsh));
    len += section;
    len += serializeJsonPretty(object, Serial);
    len += Serial.println();
    for (int i =0;i < section - 4; i++)
    {
        len += Serial.print("=");
    }
    len += Serial.println();
    return 0;
}

/**
 * Write the message to the log level.  The message is using the embedded flash support.
 * 
 *  @param level The logType level being assigned to.
 *  @param ifsh The pointer to the Flash String Helper
 *  @param object The Arduino JSON object/element to write out
 *  @return The size of the string written.
 */ 
size_t LogInfoClass::log(LogType level, const __FlashStringHelper *ifsh, JsonObjectConst object)
{
    if (level > this->_reportingLevel)
    {
        return 0;
    }
    int len = 0;
    len += writePrefix(level);
    int section = buildSectionHeader(reinterpret_cast<const char *>(ifsh));
    len += section;
    len += serializeJsonPretty(object, Serial);
    len += Serial.println();
    for (int i =0;i < section - 4; i++)
    {
        len += Serial.print("=");
    }
    len += Serial.println();
    return 0;
}

/**
 * Write the prefix header to the message
 * 
 *  @param level The logType level being assigned to.
 *  @return The size of the string written.
 */ 
size_t LogInfoClass::writePrefix(LogType level)
{
    int len = 0;
    len = Serial.printf("%10.0lu", millis());
    len += Serial.print(F(":"));
    len += Serial.print(this->logTypeToShortString(level));
    len += Serial.print(F(":"));
    len += Serial.print(xPortGetCoreID());
    len += Serial.print(F(":"));
    return len;
}

/**
 * build the message section header to the log level.
 * 
 *  @param hdr The character array to write
 *  @return The size of the string written.
 */ 
size_t LogInfoClass::buildSectionHeader(const char hdr[])
{
    int len = 0;
    len = Serial.println("");
    len += Serial.print("======= ");
    len += Serial.print(hdr);
    len += Serial.println(" =======");
    return len;
}

/**
 * Write the message to the log level. 
 * 
 *  @param level The logType level being assigned to.
 *  @param msg The character array to write
 *  @param hdr The character array to write
 *  @return The size of the string written.
 */ 
size_t LogInfoClass::write(LogType level, const char msg[], const char hdr[])
{
    if (level > this->_reportingLevel)
    {
        return 0;
    }
    int len = 0;
    len = this->writePrefix(level);
    // Check if multi line or not. 
    boolean multiline = false;
    if (strstr(msg, "\r"))
    {
        multiline = true;
        len += this->buildSectionHeader(hdr);
    }
    len += Serial.write((uint8_t*)msg, strlen(msg));
    len += Serial.println();
    if (multiline)
    {
        len += Serial.println("===========================");
    }
    Serial.flush();
    return len;
}

/**
 * Write the message to the log level.  The message has formatting information
 * 
 *  @param level The logType level being assigned to.
 *  @param format The format string that the extra parameters can be written to.
 *  @param ... parameters to be added on the message
 *  @return The size of the string written.
 */ 
size_t LogInfoClass::log(LogType level, const char *format, ...)
{
    if (level > this->_reportingLevel)
    {
        return 0;
    }
    char loc_buf[64];
    char * temp = loc_buf;

    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);

    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if (len < 0) {
        va_end(arg);
        return 0;
    };
    if (len >= sizeof(loc_buf)) {
        temp = (char*)malloc(len+1);
        if (temp == NULL) {
            va_end(arg);
            return 0;
        }
        len = vsnprintf(temp, len+1, format, arg);
    }
    va_end(arg);
    //len = this->log(level, temp);
    this->write(level, temp);
    if (temp != loc_buf) {
        free(temp);
    }
    return len;
}


LogInfoClass LogInfo;