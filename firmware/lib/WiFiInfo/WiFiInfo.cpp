
#include "esp_wps.h"
#include "WiFiInfo.h"
#include "LogInfo.h"
#include "DeviceInfo.h"
#include "LedInfo.h"
#include "NTPInfo.h"

static esp_wps_config_t config;

/**
 * Begin the initialization of the WiFi configuration
 */
void WiFiInfoClass::begin()
{
    WiFi.begin();
}

/**
 * Connect to the WiFi and display information at the screen coord passed in
 * 
 * It will connect using existing SSID detected or switch into WPA mode
 * 
 * @param x The horizontal position on the OLED screen
 * @param y The vertical position on the OLED screen
 * @return True if connected or not 
 */
bool WiFiInfoClass::connect(u8g2_uint_t x, u8g2_uint_t y)
{
    LogInfo.log(LOG_VERBOSE, F("Initialising WiFi...."));
    LedInfo.blinkOn(LED_WIFI);
    while (WiFi.status() != WL_CONNECTED)
    {
        if (this->previousMillisWiFi < this->intervalWiFi)
        {
            previousMillisWiFi = millis();
            delay(500);
        }
        else
            break;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        OledDisplay.displayLine(x, y, "WiF: %s", "waiting for WPA");
        LogInfo.log(LOG_VERBOSE, F("Not Connected so switching to STA Mode...."));
        WiFi.onEvent(WiFiInfoClass::WiFiEvent);
        WiFi.mode(WIFI_MODE_STA);
        WiFiInfoClass::wpsInitConfig();
        esp_wifi_wps_enable(&config);
        if (esp_wifi_wps_start(10) == ESP_OK)
        {
            uint16_t count = 0;
            while (WiFi.status() != WL_CONNECTED)
            {
                if (WiFi.status() == WL_CONNECTED)
                {
                    strcpy(this->_ssid, WiFi.SSID().c_str());
                    LogInfo.log(LOG_INFO, "Connected to        : %s", this->getSSID());
                    LogInfo.log(LOG_INFO, "Got IP              : %s", WiFi.localIP().toString().c_str());
                    OledDisplay.displayLine(x, y, "WiF: %s ", this->getSSID());
                    this->_connected = true;
                    LedInfo.blinkOff(LED_WIFI);
                    vTaskDelay(200);
                    LedInfo.switchOn(LED_WIFI);
                    NTPInfo.tick();
                    break;
                }
                if (millis() - this->previousMillisWiFi >= 1000)
                {
                    this->previousMillisWiFi = millis();
                    count++;
                    OledDisplay.displayLine(x + 25, y, " %i secs ", count);
                }
            }
        }
    }
    else
    {
        strcpy(this->_ssid, WiFi.SSID().c_str());
        LogInfo.log(LOG_INFO, "Connected to        : %s", WiFi.SSID().c_str());
        LogInfo.log(LOG_INFO, "Got IP              : %s", WiFi.localIP().toString().c_str());
        OledDisplay.displayLine(x, y, "WiF: %s ", this->getSSID());
        this->_connected = true;
        LedInfo.blinkOff(LED_WIFI);
        vTaskDelay(200);
        LedInfo.switchOn(LED_WIFI);
        NTPInfo.tick();
    }
    return this->getIsConnected();
}

/**
 * Is the WiFi connected flag
 * 
 * @return True if connected
 */
const bool WiFiInfoClass::getIsConnected()
{
    return this->_connected;
}

/**
 * create a JSON element that will show the current WiFi info
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */
void WiFiInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject("WiFi");
    json["ssid"] = this->getSSID();
    json["strength"] = WiFi.RSSI();
}

/**
 * Get the current SSID connect to or trying to connect to
 * 
 * @return The SSID name
 */
const char *WiFiInfoClass::getSSID()
{
    return this->_ssid;
}

/**
 * Initialize the WPS configuration
 */
void WiFiInfoClass::wpsInitConfig()
{
    config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
    config.wps_type = WPS_TYPE_PBC;
    strcpy_P(config.factory_info.manufacturer, PSTR("LUXOFT"));
    strcpy_P(config.factory_info.model_number, PSTR(DeviceInfo.getDeviceId()));
    strcpy_P(config.factory_info.model_name, PSTR("OT-1000-IOT"));
    strcpy(config.factory_info.device_name, DeviceInfo.getDeviceId());
}

/**
 * Handle the WiFi events generated when in WPS mode
 * 
 * @param event The event been raised
 * @param info The information related to the event
 */
void WiFiInfoClass::WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
    switch (event)
    {
    case SYSTEM_EVENT_STA_START:
        LogInfo.log(LOG_INFO, F("Station Mode Started"));
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        LogInfo.log(LOG_INFO, "Connected to        : %s", WiFi.SSID().c_str());
        LogInfo.log(LOG_INFO, "Got IP              : %s", WiFi.localIP().toString());
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        LogInfo.log(LOG_WARNING, F("Disconnected from station, attempting reconnection"));
        WiFi.reconnect();
        break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
        LogInfo.log(LOG_INFO, "WPS Successfull, stopping WPS and connecting to: %s", WiFi.SSID().c_str());
        esp_wifi_wps_disable();
        delay(10);
        WiFi.begin();
        LedInfo.blinkOff(LED_WIFI);
        vTaskDelay(200);
        LedInfo.switchOn(LED_WIFI);
        break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
        LogInfo.log(LOG_WARNING, F("WPS Failed, retrying"));
        esp_wifi_wps_disable();
        esp_wifi_wps_enable(&config);
        esp_wifi_wps_start(0);
        break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
        LogInfo.log(LOG_WARNING, F("WPS Timedout, retrying"));
        esp_wifi_wps_disable();
        esp_wifi_wps_enable(&config);
        esp_wifi_wps_start(0);
        break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
        LogInfo.log(LOG_VERBOSE, "WPS_PIN: %s", WiFiInfoClass::numbersToString(info.sta_er_pin.pin_code));
        break;
    default:
        break;
    }
}

/**
 * Convert the number array to a string
 * 
 * @return The WPS pin
 */
String WiFiInfoClass::numbersToString(uint8_t a[])
{
    char wps_pin[9];
    for (int i = 0; i < 8; i++)
    {
        wps_pin[i] = a[i];
    }
    wps_pin[8] = '\0';
    return (String)wps_pin;
}

WiFiInfoClass WiFiInfo;