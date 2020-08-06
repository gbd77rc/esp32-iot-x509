#include "LiDARInfo.h"
#include "Logging.h"
#include "NTPInfo.h"

RTC_DATA_ATTR int _liDAR_count;

TaskHandle_t LiDARInfoClass::readTaskHandle;
bool LiDARInfoClass::taskCreated;
SemaphoreHandle_t LiDARInfoClass::semaphoreFlag;

void LiDARInfoClass::readTask(void *parameters)
{
    for (;;)
    {
        //vTaskSuspend(LiDARInfoClass::readTaskHandle);    
        if (xSemaphoreTake(LiDARInfoClass::semaphoreFlag, portMAX_DELAY))
        {
            Logging.log(LOG_VERBOSE, "Resuming Read LiDAR Task...");    
            Logging.log(LOG_VERBOSE, "LiDAR Valid Data: %s", LiDARInfo.read() ? "Yes" : "No");
            xSemaphoreGive(LiDARInfoClass::semaphoreFlag);
        }
        delay(200);
    }
}

void LiDARInfoClass::begin()
{
    // Check if we are waking up or we have started because of manual reset or power on
    _liDAR_count = 0;
    this->_isEnabled = false;
}

void LiDARInfoClass::load(JsonObjectConst obj)
{
    this->_isEnabled = obj.containsKey("enabled") ? obj["enabled"].as<bool>() : true;
    this->_baud = obj.containsKey("baud") ? obj["baud"].as<uint32_t>() : 9600;
    this->_rxPin = obj.containsKey("rx") ? obj["rx"].as<uint16_t>() : 17;
    this->_txPin = obj.containsKey("tx") ? obj["tx"].as<uint16_t>() : 16;

    Logging.log(LOG_VERBOSE, "RX: %i TX: %i Baud: %i Enabled: %s",
        this->_rxPin, this->_txPin,
        this->_baud, this->_isEnabled ? "Yes" : "No");
}

void LiDARInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    //json["level"] = (int)this->_reportingLevel;
    json["enabled"] = this->_isEnabled;
    json["baud"] = this->_baud;
    json["rx"] = this->_rxPin;
    json["tx"] = this->_txPin;
}

void LiDARInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject("LiDAR");
    json["distance"] = this->_data.distance;
    json["strength"] = this->_data.strength;
    json["temp"] = this->_data.temperature;
    json["date_time"] = this->_last_read;
}

bool LiDARInfoClass::isConnected()
{
    return this->_isConnected;
}

bool LiDARInfoClass::connect()
{
    if (this->_isEnabled)
    {
        // SoftwareSerialConfig config = SWSERIAL_8N1;
        // this->_gpsSerial.begin(this->_baud, config, this->_rxPin, this->_txPin);
        Logging.log(LOG_VERBOSE, F("Initialising LiDAR..."));
        this->_lidarSerial.begin(this->_baud,
            SERIAL_8N1,
            this->_rxPin,
            this->_txPin);
        this->_lidar.begin(&this->_lidarSerial);

        // Set lidar options (saving is important)
        this->_lidar.set_framerate(TFMINI_PLUS_FRAMERATE_10HZ);
        this->_lidar.set_output_format(TFMINI_PLUS_OUTPUT_CM);
        this->_lidar.set_communication_interface(TFMINI_PLUS_UART);
        this->_lidar.enable_output(true);
        this->_lidar.save_settings();
    }
    if (LiDARInfoClass::taskCreated == false)
    {
        Logging.log(LOG_VERBOSE, F("Creating LiDAR Reading Task on Core 0"));
        xTaskCreatePinnedToCore(LiDARInfoClass::readTask, "ReadLiDARTask",
            10000, NULL, 1,
            &LiDARInfoClass::readTaskHandle,
            0);
        LiDARInfoClass::taskCreated = true;
        this->_isConnected = true;
        LiDARInfo.resumeTask();
    }
    Logging.log(LOG_VERBOSE, "LiDAR is running  : %s", this->_isConnected ? "Yes" : "No");
    return this->_isConnected;
}

uint16_t LiDARInfoClass::resumeTask()
{
    if (LiDARInfo.isConnected())
    {
        vTaskResume(LiDARInfoClass::readTaskHandle);
    }
    return 10000;
}

bool LiDARInfoClass::read()
{
    this->_isValid = false;
    if (this->_isEnabled)
    {
        _liDAR_count++;
        this->_isValid = this->_lidar.read_data(this->_data, true);
        if (this->_isValid)
        {
            this->_last_read = NTPInfo.getEpoch();
            Logging.log(LOG_VERBOSE,
                "Distance: %icm Strength: %i Temperature: %.2f °C",
                this->_data.distance,
                this->_data.strength,                 
                this->_data.temperature);
        }
    }
    return this->_isValid;
}

LiDARInfoClass LiDARInfo;