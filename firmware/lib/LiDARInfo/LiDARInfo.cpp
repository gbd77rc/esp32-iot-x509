#include "LiDARInfo.h"
#include "LogInfo.h"
#include "NTPInfo.h"
#include "LedInfo.h"

// const static long interval = 2000;
// TaskHandle_t LiDARInfoClass::readTaskHandle;
// bool LiDARInfoClass::taskCreated;
// long LiDARInfoClass::lastCheck;
// SemaphoreHandle_t LiDARInfoClass::semaphoreFlag;

// void LiDARInfoClass::readTask(void *parameters)
// {
//     for (;;)
//     {
//         vTaskSuspend(LiDARInfoClass::readTaskHandle);
//         if (xSemaphoreTake(LiDARInfoClass::semaphoreFlag, portMAX_DELAY))
//         {
//             LogInfo.log(LOG_VERBOSE, "Resuming Read LiDAR Task...");
//             LogInfo.log(LOG_VERBOSE, "LiDAR Valid Data: %s", LiDARInfo.read() ? "Yes" : "No");
//             xSemaphoreGive(LiDARInfoClass::semaphoreFlag);
//         }
//     }
// }

void LiDARInfoClass::begin(SemaphoreHandle_t flag)
{
    this->sensorState.semaphoreFlag = flag;
    strcpy(this->sensorState.sensorName, "LiDARInfo");
    this->sensorState.sensor = this;    
    // Check if we are waking up or we have started because of manual reset or power on
    this->_isEnabled = false;
}

void LiDARInfoClass::load(JsonObjectConst obj)
{
    this->_isEnabled = obj.containsKey("enabled") ? obj["enabled"].as<bool>() : true;
    this->_baud = obj.containsKey("baud") ? obj["baud"].as<uint32_t>() : 9600;
    this->_rxPin = obj.containsKey("rx") ? obj["rx"].as<uint16_t>() : 17;
    this->_txPin = obj.containsKey("tx") ? obj["tx"].as<uint16_t>() : 16;

    LogInfo.log(LOG_VERBOSE, "RX: %i TX: %i Baud: %i Enabled: %s",
        this->_rxPin, this->_txPin,
        this->_baud, this->_isEnabled ? "Yes" : "No");
}

void LiDARInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->getSectionName());
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
    json["cpu_temp"] = this->_data.temperature;
    json["timestamp"] = this->_last_read;
}

bool LiDARInfoClass::taskToRun() 
{
    LogInfo.log(LOG_VERBOSE, F("Reading LiDAR"));
    return true;
}

const bool LiDARInfoClass::getIsConnected()
{
    return this->_isConnected;
}

const bool LiDARInfoClass::connect()
{
    // if (this->_isEnabled)
    // {
    //     // SoftwareSerialConfig config = SWSERIAL_8N1;
    //     // this->_gpsSerial.begin(this->_baud, config, this->_rxPin, this->_txPin);
    //     LogInfo.log(LOG_VERBOSE, F("Initialising LiDAR..."));
    //     this->_lidarSerial.begin(this->_baud,
    //         SERIAL_8N1,
    //         this->_rxPin,
    //         this->_txPin);
    //     this->_lidar.begin(&this->_lidarSerial);

    //     // Set lidar options (saving is important)
    //     this->_lidar.set_framerate(TFMINI_PLUS_FRAMERATE_10HZ);
    //     this->_lidar.set_output_format(TFMINI_PLUS_OUTPUT_CM);
    //     this->_lidar.set_communication_interface(TFMINI_PLUS_UART);
    //     this->_lidar.enable_output(true);
    //     this->_lidar.save_settings();
    // }
    // if (LiDARInfoClass::taskCreated == false)
    // {
    //     LogInfo.log(LOG_VERBOSE, F("Creating LiDAR Reading Task on Core 0"));
    //     xTaskCreatePinnedToCore(LiDARInfoClass::readTask, "ReadLiDARTask",
    //         10000, NULL, 1,
    //         &LiDARInfoClass::readTaskHandle,
    //         0);
    //     LiDARInfoClass::taskCreated = true;
    //     this->_isConnected = true;
    // }
    // LogInfo.log(LOG_INFO, "LiDAR is running  : %s", this->_isConnected ? "Yes" : "No");
    LogInfo.log(LOG_VERBOSE, "Creating %s Task on Core 0", this->sensorState.sensorName);
    xTaskCreatePinnedToCore(LiDARInfoClass::task, "ReadLiDARTask",
        10000, 
        (void *)&this->sensorState, 
        1,
        &this->sensorState.taskHandle,
        0);    
    this->_isConnected = true;
    return this->_isConnected;
}

// uint16_t LiDARInfoClass::resumeTask()
// {
//     if (LiDARInfo.isConnected() && (millis() - LiDARInfoClass::lastCheck) > interval)
//     {
//         LiDARInfoClass::lastCheck = millis();
//         vTaskResume(LiDARInfoClass::readTaskHandle);
//     }
//     return 10000;
// }

bool LiDARInfoClass::read()
{
    uint16_t previous = -1;
    if (this->_isValid) {
        previous = this->_data.distance;
    }
    this->_isValid = false;
    if (this->_isEnabled)
    {
        this->_isValid = this->_lidar.read_data(this->_data, true);
        if (this->_isValid)
        {
            this->_last_read = NTPInfo.getEpoch();
            if (previous != this->_data.distance)
            {
                LogInfo.log(LOG_VERBOSE,
                    "Distance: %icm Strength: %i Temperature: %.2f °C",
                    this->_data.distance,
                    this->_data.strength,                     this->_data.temperature);
            }
        }
    }
    return this->_isValid;
}

uint16_t LiDARInfoClass::getDistance()
{
    if (this->_data.distance > 0)
    {
        return this->_data.distance;
    }
    return 0;
}

LiDARInfoClass LiDARInfo;