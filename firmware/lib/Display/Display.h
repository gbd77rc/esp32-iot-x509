#ifndef DISPLAY_H
#define DISPLAY_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <U8g2lib.h>

#define FONT_ONE_HEIGHT 8 // font one height in pixels
#define FONT_TWO_HEIGHT 20

class DisplayClass
{
public:
    void begin();
    void clear();
    void displayExit(const __FlashStringHelper *ifsh, uint8_t secondsToReboot = 10);
    void displayLine(u8g2_uint_t x, u8g2_uint_t y, const __FlashStringHelper *ifsh);    
    void displayLine(u8g2_uint_t x, u8g2_uint_t y, const char *format, ...);
private:
    char _chBuffer[768];
    void writeLine(u8g2_uint_t x, u8g2_uint_t y, const char *msg);
};

extern DisplayClass OledDisplay;

#endif
