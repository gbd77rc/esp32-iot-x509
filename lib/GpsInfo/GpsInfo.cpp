#include "GpsInfo.h"
#include "Logging.h"
//#include "SoftwareSerial.h"
#include "HardwareSerial.h"
#include "NTPInfo.h"

RTC_DATA_ATTR int _gps_count;

TaskHandle_t GpsInfoClass::readTaskHandle;
bool GpsInfoClass::taskCreated;
SemaphoreHandle_t GpsInfoClass::semaphoreFlag;

void GpsInfoClass::readTask(void *parameters)
{
    for (;;)
    {
        vTaskSuspend(GpsInfoClass::readTaskHandle);            
        if (xSemaphoreTake(GpsInfoClass::semaphoreFlag, portMAX_DELAY))
        {
            Logging.log(LOG_VERBOSE, "Resuming Read GPS Task..."); 
            if (GpsInfo.isConnected())
            {
                Logging.log(LOG_VERBOSE, "GPS Valid Data: %s", GpsInfo.read() ? "Yes" : "No");
            }
            xSemaphoreGive(GpsInfoClass::semaphoreFlag);
        }
    }
}

uint16_t GpsInfoClass::resumeTask()
{
    if (GpsInfo.isConnected())
    {
        vTaskResume(GpsInfoClass::readTaskHandle);
    }
    return 10000;
}

void GpsInfoClass::begin()
{
    // Check if we are waking up or we have started because of manual reset or power on
    // if (WakeUp.isPoweredOn())
    // {
         _gps_count = 0;
    // }
    this->_isEnabled = false;
}

void GpsInfoClass::load(JsonObjectConst obj)
{
    this->_isEnabled = obj.containsKey("enabled") ? obj["enabled"].as<bool>() : true;
    this->_baud = obj.containsKey("baud") ? obj["baud"].as<uint32_t>() : 9600;
    this->_rxPin = obj.containsKey("rx") ? obj["rx"].as<uint16_t>() : 17;
    this->_txPin = obj.containsKey("tx") ? obj["tx"].as<uint16_t>() : 16;
    Logging.log(LOG_VERBOSE, "RX: %i TX: %i Baud: %i Enabled: %s",
        this->_rxPin, this->_txPin,
        this->_baud, this->_isEnabled ? "Yes" : "No");    
}

void GpsInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    //json["level"] = (int)this->_reportingLevel;
    json["enabled"] = this->_isEnabled;
    json["baud"] = this->_baud;
    json["rx"] = this->_rxPin;
    json["tx"] = this->_txPin;
}

void GpsInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject("GPS");
    auto loc = json.createNestedObject("location");
    loc["isValid"] = this->_gps.location.isValid();
    loc["longitude"] = this->_gps.location.lng();
    loc["latitude"] = this->_gps.location.lat();
    loc["altitude"] = this->_gps.altitude.meters();
    loc["age"] = this->_gps.location.age();
    char buffer[20];
    if (this->_gps.time.isValid())
    {
        snprintf(buffer, 12, "%02i:%02i:%02iZ",
                 this->_gps.time.hour(),
                 this->_gps.time.minute(),
                 this->_gps.time.second());
        json["time"] = buffer;
    }
    json["date_time"] = this->_last_read;
}

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
    coords.add(this->_gps.location.lng());
    coords.add(this->_gps.location.lat());
    coords.add(this->_gps.altitude.meters());
    auto props = feature.createNestedObject("properties");
    props["gps_valid"] = this->_gps.location.isValid();
    char buffer[20];
    if (this->_gps.time.isValid())
    {
        snprintf(buffer, 12, "%02i:%02i:%02iZ",
                 this->_gps.time.hour(),
                 this->_gps.time.minute(),
                 this->_gps.time.second());
        props["sat_time"] = buffer;
    }
    props["last_epoch"] = this->_last_read;
}

bool GpsInfoClass::isConnected()
{
    return this->_isConnected;
}

bool GpsInfoClass::connect()
{
    if (this->_isEnabled)
    {
        // SoftwareSerialConfig config = SWSERIAL_8N1;
        // this->_gpsSerial.begin(this->_baud, config, this->_rxPin, this->_txPin);

        this->_gpsSerial.begin(this->_baud,
                               SERIAL_8N1,
                               this->_rxPin,
                               this->_txPin);

        this->_isConnected = this->read();
    }
    if (GpsInfoClass::taskCreated == false)
    {
        Logging.log(LOG_VERBOSE, F("Creating GPS Reading Task on Core 0"));
        xTaskCreatePinnedToCore(GpsInfoClass::readTask, "ReadGpsTask",
                                10000, NULL, 1,
                                &GpsInfoClass::readTaskHandle,
                                0);
        GpsInfoClass::taskCreated = true;
        this->_isConnected = true;
        GpsInfo.resumeTask();
    }
    Logging.log(LOG_VERBOSE, "GPS is connected : %s", this->_isConnected ? "Yes" : "No");
    return this->_isConnected;
}

bool GpsInfoClass::read()
{
    if (this->_isEnabled)
    {
        int count = 0;
        uint64_t now = millis();
        while (true)
        {
            while (this->_gpsSerial.available() > 0)
            {
                count = this->_gps.encode(this->_gpsSerial.read());
            }
            if ((millis() - now) > 5000)
            {
                Logging.log(LOG_WARNING, F("Have not received valid GPS data in the last 5 seconds"));
                break;
            }
            if (this->_gps.location.isValid() && this->_gps.location.isUpdated())
            {
                this->_last_read = NTPInfo.getEpoch();
                break;
            }
        }
        Logging.log(LOG_VERBOSE, "GPS is valid %s  - Read %i Is Updated: %s",
                    this->_gps.location.isValid() ? "Yes" : "No", count,
                    this->_gps.location.isUpdated() ? "Yes" : "No");
        Logging.log(LOG_VERBOSE, "Time is valid: %s Updated: %s",
                    this->_gps.time.isValid() ? "Yes" : "No",
                    this->_gps.time.isUpdated() ? "Yes" : "No");
        Logging.log(LOG_VERBOSE, "GPS = %s", this->location());
    }
    return this->_gps.location.isValid();
}

const char *GpsInfoClass::location()
{
    if (this->_isConnected && this->_gps.location.isValid())
    {
        snprintf(this->_location, 50, "Lat/Lng : %8.5f,%8.5f",
                 this->_gps.location.lat(),
                 this->_gps.location.lng());
    }
    else
    {
        strcpy(this->_location, "Lat/Lng : UNKNOWN");
    }
    return this->_location;
}

GpsInfoClass GpsInfo;