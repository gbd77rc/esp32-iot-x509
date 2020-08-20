#include "NTPInfo.h"

/**
 * Begin the initialization of the NTP system
 */
void NTPInfoClass::begin()
{
    this->_ntp.begin();
    this->_ntp.update();
}

/**
 * Get the formatted date string
 * 
 * Example: 2020-01-01
 * 
 * @return Formmatted date string.
 */
String NTPInfoClass::getFormattedDate() // Convert epoch time to date - Surprising not part of NTPClient Package!
{
    unsigned long rawTime = this->_ntp.getEpochTime() / 86400L; // in days
    unsigned long days = 0, year = 1970;
    uint8_t month;
    static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    while ((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
        year++;

    rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
    days = 0;
    for (month = 0; month < 12; month++)
    {
        uint8_t monthLength;
        if (month == 1)
        { // february
            monthLength = LEAP_YEAR(year) ? 29 : 28;
        }
        else
        {
            monthLength = monthDays[month];
        }
        if (rawTime < monthLength)
            break;
        rawTime -= monthLength;
    }
    String monthStr = ++month < 10 ? "0" + String(month) : String(month);     // jan is month 1
    String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month
    return String(year) + "-" + monthStr + "-" + dayStr;
}

/**
 * Get the formatted iso8601 date/time string
 * 
 * Example: 2020-01-01T13:45:09Z
 * 
 * @return Formmatted iso8601 date/time string
 */
String NTPInfoClass::getISO8601Formatted() // Convert epoch time to ISO8601 formatted date/time
{
    String date = this->getFormattedDate();
    return date + String("T") + this->_ntp.getFormattedTime() + "Z";
}

/**
 * Get the formatted time string
 * 
 * Example: 13:45:09
 * 
 * @return Formmatted time string
 */
String NTPInfoClass::getFormattedTime() // Get the formatted time from the generic NTP Client
{
    return this->_ntp.getFormattedTime();
}

/**
 * Get the latest time from the internet
 */
void NTPInfoClass::tick() // Call the NTP update function to pool the site
{
    int8_t count = 0;
    long epoch = this->getEpoch();
    do
    {
        count++;
        if ( count > 10)
        {
            break;
        }
        this->_ntp.update();
        delay(10);
        epoch = this->getEpoch();
    } while (epoch < 1577836800);
}

/**
 * Get the current Epoch time in seconds from 1970-01-01
 * 
 * Example: 1597740349
 * 
 * @return Epoch timestamp
 */
long NTPInfoClass::getEpoch() // Get the current epoch time
{
    long epoch = this->_ntp.getEpochTime();
    if (epoch > 1577836800)
    {
        return epoch;
    }
    return millis();
}

NTPInfoClass NTPInfo; // Single instance declaration
