#ifndef LOGGING_H
#define LOGGING_H
#include <Arduino.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include "Config.h"

typedef enum
{
    LOG_OFF = 0,
    LOG_ERROR = 1,
    LOG_WARNING = 2,
    LOG_INFO = 3,
    LOG_VERBOSE = 4,
    LOG_ALL = 5
} LogType;

class LoggingClass: public BaseConfigInfoClass
{
    public:
    LoggingClass(): BaseConfigInfoClass("logging"), _reportingLevel(LOG_ALL) {}
    void begin();
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    void toJson(JsonObject ob) override;    

    const char* getUniqueId();
    size_t log(LogType level, const char *format, ...);
    size_t log(LogType level, const __FlashStringHelper *ifsh);
    size_t log(LogType level, const __FlashStringHelper *ifsh, JsonObject object);
    size_t log(LogType level, const __FlashStringHelper *ifsh, JsonObjectConst object);
    void setLogLevel(LogType logType);
    void setLogLevel(const char* logType);
    const char* getLogLevel();
    private:
    const char* logTypeToShortString(LogType level);
    const char* logTypeToString(LogType level);
    LogType stringToLogType(const char* level);      
    LogType _reportingLevel;
    size_t write(LogType level, const char msg[], const char hdr[] = "Information" );
    size_t writePrefix(LogType level);
    size_t writeSectionHeader(const char hdr[]);
    char _uniqueId[23];
};

extern LoggingClass Logging;

#endif