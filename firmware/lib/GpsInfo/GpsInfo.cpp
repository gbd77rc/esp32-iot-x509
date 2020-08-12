#include "GpsInfo.h"
#include "LogInfo.h"
#include "NTPInfo.h"
#include "LedInfo.h"

RTC_DATA_ATTR int _gps_count;
const long interval = 5000;
int16_t POSITION_SIZE = 128;
TaskHandle_t GpsInfoClass::readTaskHandle;
bool GpsInfoClass::taskCreated;
long GpsInfoClass::lastCheck;
SemaphoreHandle_t GpsInfoClass::semaphoreFlag;

void GpsInfoClass::readTask(void *parameters)
{
    for (;;)
    {
        vTaskSuspend(GpsInfoClass::readTaskHandle);
        if (xSemaphoreTake(GpsInfoClass::semaphoreFlag, portMAX_DELAY))
        {
            LogInfo.log(LOG_VERBOSE, "Resuming Read GPS Task...");
            if (GpsInfo.isConnected())
            {
                LogInfo.log(LOG_VERBOSE, "GPS Valid Data: %s", GpsInfo.read() ? "Yes" : "No");
            }
            GpsInfoClass::lastCheck = millis();
            xSemaphoreGive(GpsInfoClass::semaphoreFlag);
        }
    }
}

uint16_t GpsInfoClass::resumeTask()
{
    if (GpsInfo.isConnected() && (millis() - GpsInfoClass::lastCheck) > interval)
    {
        //GpsInfoClass::lastCheck = millis();
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
    this->_isSim808 = obj.containsKey("sim808") ? obj["sim808"].as<bool>(): false;
    LogInfo.log(LOG_VERBOSE, "RX: %i TX: %i Baud: %i Enabled: %s Sim808: %s",
        this->_rxPin, this->_txPin,
        this->_baud, this->_isEnabled ? "Yes" : "No",
        this->_isSim808  ? "Yes": "No");
}

void GpsInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    //json["level"] = (int)this->_reportingLevel;
    json["enabled"] = this->_isEnabled;
    json["baud"] = this->_baud;
    json["rx"] = this->_rxPin;
    json["tx"] = this->_txPin;
    json["sim808"] = this->_isSim808;
}

void GpsInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject("GPS");
    auto loc = json.createNestedObject("location");
    loc["longitude"] = this->_long;
    loc["latitude"] = this->_lat;
    loc["satelites"] = this->_satelites;
    loc["course"] = this->_course;
    loc["speed"] = this->_speed;
    loc["altitude"] = this->_altitude;
    loc["timestamp"] = this->_last_read;
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
    coords.add(this->_long);
    coords.add(this->_lat);
    coords.add(this->_altitude);
    auto props = feature.createNestedObject("properties");
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

        if (this->_isSim808)
        {
            this->_sim.begin(this->_gpsSerial);
            //this->_sim.powerOnOff(true);
            LogInfo.log(LOG_VERBOSE, "Initialising SIM808....");
            //this->_sim.init();
            LogInfo.log(LOG_VERBOSE, "Powering ON GPS....");
            this->_sim.powerOnOffGps(true);
            bool isPowered = false;
            this->_sim.getGpsPowerState(&isPowered);
            LogInfo.log(LOG_VERBOSE, "Completed SIM808....%s",  isPowered ? "Yes" : "No");
        }

        this->_isConnected = this->read();
    }
    if (GpsInfoClass::taskCreated == false)
    {
        LogInfo.log(LOG_VERBOSE, F("Creating GPS Reading Task on Core 0"));
        xTaskCreatePinnedToCore(GpsInfoClass::readTask, "ReadGpsTask",
            10000, NULL, 1,
            &GpsInfoClass::readTaskHandle,
            0);
        GpsInfoClass::taskCreated = true;
        this->_isConnected = true;
        GpsInfo.resumeTask();
    }
    LogInfo.log(LOG_VERBOSE, "GPS is connected : %s", this->_isConnected ? "Yes" : "No");
    return this->_isConnected;
}

bool GpsInfoClass::read()
{
    if (this->_isEnabled)
    {
        LedInfo.switchOn(LED_GPS);
        int count = 0;
        uint64_t now = millis();
        this->_isValid = false;
        char position[POSITION_SIZE];
        while (true)
        {
            if (this->_isSim808)
            {
                SIM808GpsStatus status = this->_sim.getGpsStatus(position, POSITION_SIZE);
                if ((millis() - now) > 5000)
                {
                    LogInfo.log(LOG_WARNING, F("Have not received valid GPS data in the last 5 seconds"));
                    LogInfo.log(LOG_VERBOSE, "Count read is %i", count);
                    break;
                }
                if (status < SIM808GpsStatus::Fix) {
                    continue;
                }
                this->_sim.getGpsField(position, SIM808GpsField::GnssUsed, &this->_satelites);
                this->_sim.getGpsField(position, SIM808GpsField::Latitude, &this->_lat);
                this->_sim.getGpsField(position, SIM808GpsField::Longitude, &this->_long);
                this->_sim.getGpsField(position, SIM808GpsField::Course, &this->_course);
                this->_sim.getGpsField(position, SIM808GpsField::Speed, &this->_speed);
                this->_sim.getGpsField(position, SIM808GpsField::Altitude, &this->_altitude);
                break;
            }
            else
            {
                TinyGPSPlus gps = TinyGPSPlus();
                while (this->_gpsSerial.available() > 0)
                {
                    count = gps.encode(this->_gpsSerial.read());
                }
                if ((millis() - now) > 5000)
                {
                    LogInfo.log(LOG_WARNING, F("Have not received valid GPS data in the last 5 seconds"));
                    LogInfo.log(LOG_VERBOSE, "Count read is %i", count);
                    break;
                }
                if (gps.location.isValid() && gps.location.isUpdated())
                {
                    this->_last_read = NTPInfo.getEpoch();
                    break;
                }
                if (gps.location.isValid())
                {
                    this->_long = gps.location.lng();
                    this->_lat = gps.location.lat();
                    this->_altitude = gps.altitude.meters();
                    this->_course = gps.course.value();
                    this->_speed = gps.speed.value();
                    this->_satelites = gps.satellites.value();
                    this->_isValid = true;
                    LogInfo.log(LOG_VERBOSE, "GPS = Lat/Lng %s", this->location());
                }
            }
            vTaskDelay(50);
        }
        LedInfo.switchOff(LED_GPS);
    }
    return this->_isValid;
}

const char *GpsInfoClass::location()
{
    if (this->_isConnected && this->_isValid)
    {
        snprintf(this->_location, 50, "Lat/Lng: %8.5f,%8.5f",
            this->_lat,
            this->_long);
    }
    else
    {
        strcpy(this->_location, "UNKNOWN");
    }
    return this->_location;
}

float GpsInfoClass::getLong()
{
    return this->_isValid ? this->_long : 0.0;
}

float GpsInfoClass::getLat()
{
    return this->_isValid ? this->_lat : 0.0;
}

uint16_t GpsInfoClass::getSatelites()
{
    return this->_isValid ? this->_satelites : 0;
}

GpsInfoClass GpsInfo;