#ifndef NTPINFO_H
#define NTPINFO_H
#include <Arduino.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <WiFiUdp.h>  
#include <NTPClient.h>

#ifndef LEAP_YEAR
#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )
#endif

class NTPInfoClass 
{
    public:
        NTPInfoClass();
        void begin();
        String getFormattedDate();
        String getISO8601Formatted();
        String getFormattedTime();
        void tick();
        long getEpoch();
    private:
        NTPClient _ntp;
};

extern NTPInfoClass NTPInfo;

#endif