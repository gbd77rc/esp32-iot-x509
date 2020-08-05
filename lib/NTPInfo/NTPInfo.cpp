#include "NTPInfo.h"

WiFiUDP ntpUDP;

NTPInfoClass::NTPInfoClass()
    :_ntp(ntpUDP, "pool.ntp.org") // Default NTP site
    {}

void NTPInfoClass::begin()
{
    this->_ntp.begin();
    this->_ntp.update();
}

String NTPInfoClass::getFormattedDate()  // Convert epoch time to date - Surprising not part of NTPClient Package!
{
    unsigned long rawTime = this->_ntp.getEpochTime() / 86400L;  // in days
    unsigned long days = 0, year = 1970;
    uint8_t month;
    static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

    while((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
        year++;
      
    rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
    days=0;
    for (month=0; month<12; month++) {
        uint8_t monthLength;
        if (month==1) 
        { // february
            monthLength = LEAP_YEAR(year) ? 29 : 28;
        } else {
            monthLength = monthDays[month];
        }
        if (rawTime < monthLength) break;
        rawTime -= monthLength;
    }
    String monthStr = ++month < 10 ? "0" + String(month) : String(month); // jan is month 1  
    String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month  
    return String(year) + "-" + monthStr + "-" + dayStr;    
}

String NTPInfoClass::getISO8601Formatted()   // Convert epoch time to ISO8601 formatted date/time
{
    String date = this->getFormattedDate();
    return date + String("T") + this->_ntp.getFormattedTime() + "Z";
}

String NTPInfoClass::getFormattedTime()      // Get the formatted time from the generic NTP Client
{
    return this->_ntp.getFormattedTime();
}

void NTPInfoClass::tick()                  // Call the NTP update function to pool the site
{
    this->_ntp.update();
}

long NTPInfoClass::getEpoch()                // Get the current epoch time
{
    return this->_ntp.getEpochTime();
}

NTPInfoClass NTPInfo;                     // Single instance declaration
