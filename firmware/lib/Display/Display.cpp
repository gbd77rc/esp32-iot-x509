#include "Display.h"
#include "LogInfo.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 16, 15, 4);

/**
 * Begin the initialization of the OLED Screen
 */
void DisplayClass::begin()
{
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();
}

/**
 * Clear the internal buffer
 */
void DisplayClass::clear()
{
    u8g2.clearBuffer();
}

/**
 * Display an Error Message and reset ESP32
 * 
 * @param ifsh Flash Helper String
 */
void DisplayClass::displayExit(const __FlashStringHelper *ifsh)
{
    u8g2.clearDisplay();
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();
    LogInfo.log(LOG_ERROR, ifsh);
    u8g2.setDrawColor(1);
    sprintf(this->_chBuffer, "%s", reinterpret_cast<const char *>(ifsh));
    u8g2.drawStr(64 - (u8g2.getStrWidth(this->_chBuffer) / 2), 0, this->_chBuffer);
    sprintf(this->_chBuffer, "%s", reinterpret_cast<const char *>(F("Restarting in 10 seconds!")));
    u8g2.drawStr(64 - (u8g2.getStrWidth(this->_chBuffer) / 2), 15, this->_chBuffer);
    u8g2.sendBuffer();
    for (uint8_t i = 10; i > 0; i--)
    {
        delay(1000);
        sprintf(this->_chBuffer, "Restarting in %i  seconds!", i);
        u8g2.drawStr(64 - (u8g2.getStrWidth(this->_chBuffer) / 2), 15, this->_chBuffer);
        u8g2.sendBuffer();
    }
    ESP.restart();
}

/**
 * Display Flash String at x,y
 * 
 * @param x vertical position in pixels
 * @param y horizontal position in pixels
 * @param ifsh  flash string to be displayed
 */
void DisplayClass::displayLine(u8g2_uint_t x, u8g2_uint_t y, const __FlashStringHelper *ifsh)
{
    this->writeLine(x, y, reinterpret_cast<const char *>(ifsh));
}

/**
 * Display string at x,y
 * 
 * @param x vertical position in pixels
 * @param y horizontal position in pixels
 * @param format The format string that the extra parameters can be written to.
 * @param ... parameters to be added on the message 
 */
void DisplayClass::displayLine(u8g2_uint_t x, u8g2_uint_t y, const char *format, ...)
{
    char * temp = this->_chBuffer;

    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);

    int len = vsnprintf(temp, sizeof(this->_chBuffer), format, copy);
    va_end(copy);
    if (len < 0) {
        va_end(arg);
        return;
    };
    if (len >= sizeof(this->_chBuffer)) {
        temp = (char*)malloc(len+1);
        if (temp == NULL) {
            va_end(arg);
            return;
        }
        len = vsnprintf(temp, len+1, format, arg);
    }
    va_end(arg);
    this->writeLine(x, y, temp);

    if (temp != this->_chBuffer) {
        free(temp);
    }
}

/**
 * Write the actual msg to screen
 * 
 * @param x vertical position in pixels
 * @param y horizontal position in pixels
 * @param msg The string that is written
 */
void DisplayClass::writeLine(u8g2_uint_t x, u8g2_uint_t y, const char *msg)
{
    u8g2.setDrawColor(0);
    u8g2_uint_t yt = y-(FONT_ONE_HEIGHT / 2);
    if (yt < 0)
    {
        yt = FONT_ONE_HEIGHT / 2;
    }
    u8g2.drawBox(x, yt, 128, (FONT_ONE_HEIGHT + 1));
    u8g2.setDrawColor(1);
    u8g2.drawStr(x, yt, msg);
    u8g2.sendBuffer();
}

DisplayClass OledDisplay;