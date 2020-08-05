#include "Logging.h"

void LoggingClass::begin()
{
    uint64_t chipid;

    chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
    snprintf(this->_uniqueId, 23, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);   
}

void LoggingClass::load(JsonObjectConst obj)
{
    this->setLogLevel(obj["level"].as<const char*>());
    this->log(LOG_VERBOSE, "Chip Id: %s", this->getUniqueId()); 
    this->_changed = false;
}

void LoggingClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    json["level"] = logTypeToString(this->_reportingLevel);
}

void LoggingClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject(this->sectionName());
    json["level"] = logTypeToString(this->_reportingLevel);
}

const char* LoggingClass::getUniqueId()
{
    return this->_uniqueId;
}

const char* LoggingClass::logTypeToShortString(LogType level)
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

const char* LoggingClass::logTypeToString(LogType level)
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

LogType LoggingClass::stringToLogType(const char* level)
{
    if(strcasecmp(level, "OFF") == 0)
    {
        return LOG_OFF;
    }
    if(strcasecmp(level, "ERROR") == 0)
    {
        return LOG_ERROR;
    }
    if(strcasecmp(level, "WARNING") == 0)
    {
        return LOG_WARNING;
    }
    if(strcasecmp(level, "INFO") == 0)
    {
        return LOG_INFO;
    }
    if(strcasecmp(level, "VERBOSE") == 0)
    {
        return LOG_VERBOSE;
    }
    if(strcasecmp(level, "ALL") == 0)
    {
        return LOG_ALL;
    }    
    return LOG_WARNING;
}

void LoggingClass::setLogLevel(LogType logType)
{
    this->_reportingLevel = logType;
}

void LoggingClass::setLogLevel(const char* logType)
{
    this->_reportingLevel = LoggingClass::stringToLogType(logType);
}

const char* LoggingClass::getLogLevel()
{
    return LoggingClass::logTypeToString(this->_reportingLevel);
}

size_t LoggingClass::log(LogType level, const __FlashStringHelper *ifsh)
{
    return this->write(level, reinterpret_cast<const char *>(ifsh));
}

size_t LoggingClass::log(LogType level, const __FlashStringHelper *ifsh, JsonObject object)
{
    if (level > this->_reportingLevel)
    {
        return 0;
    }
    int len = 0;
    len += writePrefix(level);
    int section = writeSectionHeader(reinterpret_cast<const char *>(ifsh));
    len += section;
    len += serializeJsonPretty(object, Serial);
    len += Serial.println();
    for(int i =0;i < section - 4; i++)
    {
        len += Serial.print("=");
    }
    len += Serial.println();
    return 0;
}

size_t LoggingClass::log(LogType level, const __FlashStringHelper *ifsh, JsonObjectConst object)
{
    if (level > this->_reportingLevel)
    {
        return 0;
    }
    int len = 0;
    len += writePrefix(level);
    int section = writeSectionHeader(reinterpret_cast<const char *>(ifsh));
    len += section;
    len += serializeJsonPretty(object, Serial);
    len += Serial.println();
    for(int i =0;i < section - 4; i++)
    {
        len += Serial.print("=");
    }
    len += Serial.println();
    return 0;
}

size_t LoggingClass::writePrefix(LogType level)
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

size_t LoggingClass::writeSectionHeader(const char hdr[])
{
    int len = 0;
    len = Serial.println("");
    len += Serial.print("======= ");
    len += Serial.print( hdr );
    len += Serial.println(" =======");
    return len;
}

size_t LoggingClass::write(LogType level, const char msg[], const char hdr[])
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
        len += this->writeSectionHeader(hdr);
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

size_t LoggingClass::log(LogType level, const char *format, ...)
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
    if(len < 0) {
        va_end(arg);
        return 0;
    };
    if(len >= sizeof(loc_buf)){
        temp = (char*) malloc(len+1);
        if(temp == NULL) {
            va_end(arg);
            return 0;
        }
        len = vsnprintf(temp, len+1, format, arg);
    }
    va_end(arg);
    //len = this->log(level, temp);
    this->write(level, temp);
    if(temp != loc_buf){
        free(temp);
    }
    return len;    
}


LoggingClass Logging;