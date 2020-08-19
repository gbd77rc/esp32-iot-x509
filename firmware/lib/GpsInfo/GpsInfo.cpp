#include "GpsInfo.h"
#include "LogInfo.h"
#include "NTPInfo.h"
#include "WakeUpInfo.h"

RTC_DATA_ATTR int _gpsCount;

/**
 * overridden begin method to initialise the gp sensor and assign the semaphore flag
 * 
 * @param flag The semaphore flag to use
 */
void GpsInfoClass::begin(SemaphoreHandle_t flag)
{
    this->_semaphoreFlag = flag;
    // Check if we are waking up or we have started because of manual reset or power on
    if (WakeUp.isPoweredOn())
    {
        _gpsCount = 0;
    }
}

/**
 * overridden load JSON element into the GpsSensor instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void GpsInfoClass::load(JsonObjectConst obj)
{
    this->_enabled = obj.containsKey("enabled") ? obj["enabled"].as<bool>() : true;
    this->_baud = obj.containsKey("baud") ? obj["baud"].as<uint32_t>() : 9600;
    this->_rxPin = obj.containsKey("rx") ? obj["rx"].as<uint16_t>() : 22;
    this->_txPin = obj.containsKey("tx") ? obj["tx"].as<uint16_t>() : 23;
    this->_sampleRate = obj.containsKey("sampleRate") ? obj["sampleRate"].as<int>() : 1000;
    LogInfo.log(LOG_VERBOSE, "GPS RX: %i TX: %i Baud: %i Enabled: %s",
                this->_rxPin, this->_txPin,
                this->_baud, this->getIsEnabled() ? "Yes" : "No");
    this->_gpsSerial.begin(this->_baud,
                           SERIAL_8N1,
                           this->_rxPin,
                           this->_txPin);
}

/**
 * overridden save JSON element from the GpsSensor instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void GpsInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    json["enabled"] = this->getIsEnabled();
    json["baud"] = this->_baud;
    json["rx"] = this->_rxPin;
    json["tx"] = this->_txPin;
}

/**
 * overridden create a JSON element that will show the current GpsSensor telemetry
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */
void GpsInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject("GPSSensor");
    auto loc = json.createNestedObject("location");
    loc["longitude"] = this->_long;
    loc["latitude"] = this->_lat;
    loc["satellites"] = this->_satelites;
    loc["course"] = this->_course;
    loc["speed"] = this->_speed;
    loc["altitude"] = this->_altitude;
    loc["timestamp"] = this->_epoch_time;
}

/**
 * overridden create a JSON element that will show the current GpsSensor telemetry in GeoJson format
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */
void GpsInfoClass::toGeoJson(JsonObject ob)
{
    auto json = ob.createNestedObject("GPS");
    json["type"] = "FeatureCollection";
    auto fc = json.createNestedArray("features");

    auto feature = fc.createNestedObject();
    feature["type"] = "Feature";
    auto loc = feature.createNestedObject("geometry");
    loc["type"] = "Point";
    auto coords = loc.createNestedArray("coordinates");
    coords.add(this->_long);
    coords.add(this->_lat);
    coords.add(this->_altitude);
    auto props = feature.createNestedObject("properties");
    props["last_epoch"] = this->_epoch_time;
}

/**
 * overridden task that will be ran every n sample rate
 * 
 * @return True if successfully read the GPS sensor
 */
bool GpsInfoClass::taskToRun()
{
    if (this->getIsEnabled())
    {
        int count = 0;
        int retries = 0;
        uint64_t now = millis();
        this->_isValid = false;
        TinyGPSPlus gps = TinyGPSPlus();
        while (true)
        {
            while (this->_gpsSerial.available() > 0)
            {
                count = gps.encode(this->_gpsSerial.read());
            }
            vTaskDelay(30);
            if ((millis() - now) > 5000)
            {
                LogInfo.log(LOG_WARNING, F("Have not received valid GPS data in the last 5 seconds"));
                LogInfo.log(LOG_VERBOSE, "Count read is %i", count);
                retries++;
                now = millis();
                vTaskDelay(100);
                if (retries > 10)
                {
                    LogInfo.log(LOG_WARNING, "Tried at least 10 times to get GPS and Failed");
                    break;
                }
            }
            if (gps.location.isValid())
            {
                this->_long = gps.location.lng();
                this->_lat = gps.location.lat();
                this->_course = gps.course.value();
                this->_speed = gps.speed.value();
                if (gps.altitude.isUpdated())
                {
                    this->_altitude = gps.altitude.meters();
                }
                if (gps.satellites.isUpdated())
                {
                    this->_satelites = gps.satellites.value();
                }
                this->_isValid = true;
                this->_connected = true;
                LogInfo.log(LOG_VERBOSE, "GPS = %s", this->toString());
                _gpsCount++;
                vTaskDelay(100);
                break;
            }
        }
        this->_last_read = millis();
    }
    return this->_isValid;
}

/**
 * overridden connect to the sensor and see it is working or not
 * 
 * @return True if successfully connect
 */
const bool GpsInfoClass::connect()
{
    if (this->taskToRun())
    {
        LogInfo.log(LOG_VERBOSE, "Creating %s Task on Core 0", this->getName());
        xTaskCreatePinnedToCore(GpsInfoClass::task, "ReadGpsTask",
                                10000,
                                (void *)&this->_instance,
                                1,
                                &this->_taskHandle,
                                0);
        this->_connected = true;
    }
    LogInfo.log(LOG_VERBOSE, "GPS is connected : %s", this->getIsConnected() ? "Yes" : "No");
    return this->_connected;
}

/**
 * get display friendly info
 * 
 * @return The buffer pointer
 */
const char *GpsInfoClass::toString()
{
    if (this->getIsConnected() && this->_isValid)
    {
        snprintf(this->_location, 50, "%0.4f,%0.4f",
                 this->_lat,
                 this->_long);
    }
    else
    {
        strcpy(this->_location, "UNKNOWN");
    }
    return this->_location;
}

/**
 * override update the enabled flag and update the configuration has changed flag
 * 
 * @param flag Is enabled true or false now
 */
void GpsInfoClass::changeEnabled(bool flag)
{
    if (flag != this->getIsEnabled())
    {
        this->_enabled = flag;
        this->_changed = true;
    }
}

GpsInfoClass GpsSensor;