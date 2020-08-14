#include "LogInfo.h"
#include "EnvSensor.h"
#include "NTPInfo.h"
#include "WakeUpInfo.h"

RTC_DATA_ATTR int _count;

TaskHandle_t EnvSensorClass::readTaskHandle;
bool EnvSensorClass::taskCreated;
SemaphoreHandle_t EnvSensorClass::semaphoreFlag;

uint16_t EnvSensorClass::resumeTask()
{
    if (EnvSensor.isConnected())
    {
        vTaskResume(EnvSensorClass::readTaskHandle);
    }
    return 5000;
}

void EnvSensorClass::readTask(void *parameters)
{
    for (;;)
    {
        vTaskSuspend(EnvSensorClass::readTaskHandle);
        if (xSemaphoreTake(EnvSensorClass::semaphoreFlag, portMAX_DELAY))
        {
            Logging.log(LOG_VERBOSE, "Resuming Read Env Task...");
            if (EnvSensor.isConnected())
            {
                EnvSensor.read();
            }
            xSemaphoreGive(EnvSensorClass::semaphoreFlag);
        }
    }
}

uint16_t updateInfo()
{
    EnvSensor.displayInfo();
    return 5000;
}

void EnvSensorClass::begin()
{
    // Check if we are waking up or we have started because of manual reset or power on
    if (WakeUp.isPoweredOn())
    {
        _count = 0;
    }

    this->_humidity = 0.0;
    this->_pressure = 0.0;
    this->_temperature = 0.0;
    this->_connected = false;
}

void EnvSensorClass::load(JsonObjectConst obj)
{
    int scale = obj["scale"].as<int>();
    if (scale == 0)
    {
        this->_scale = ENV_CELSIUS;
    }
    else
    {
        this->_scale = (ScaleType)obj["scale"].as<int>();
    }

    switch (this->_scale)
    {
    case ENV_CELSIUS:
        strcpy(this->_symbol, "C");
        break;
    case ENV_FAHRENHEIT:
        strcpy(this->_symbol, "F");
        break;
    case ENV_KELVIN:
        strcpy(this->_symbol, "K");
        break;
    };
    this->_testOnly = obj["test_mode"].as<bool>();

    this->_id = 92;
    this->_address = 118;
}

bool EnvSensorClass::connect()
{
    this->_connected = bme.begin(this->_address);

    if (this->_connected == false)
    {
        Logging.log(LOG_ERROR, F("Could not find a valid BMP280 sensor, check the wiring!"));
    }
    else
    {
        this->_canRead = true;
    }
    if (EnvSensorClass::taskCreated == false && this->_connected)
    {
        Logging.log(LOG_VERBOSE, F("Creating Environment Reading Task on Core 0"));
        xTaskCreatePinnedToCore(EnvSensorClass::readTask, "ReadEnvTask",
                                10000, NULL, 1,
                                &EnvSensorClass::readTaskHandle,
                                0);
        EnvSensorClass::taskCreated = true;
        ez.addEvent(EnvSensorClass::resumeTask, 5000);
    }
    Logging.log(LOG_VERBOSE, "Env Sensor is connected : %s", this->_connected ? "Yes" : "No");
    return this->_connected;
}

void EnvSensorClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    //json["level"] = (int)this->_reportingLevel;
    json["ic2_id"] = this->_id;
    json["address"] = this->_address;
    json["scale"] = (int)this->_scale;
    json["test_mode"] = this->_testOnly;
}

bool EnvSensorClass::isConnected()
{
    return this->_connected;
}

boolean EnvSensorClass::read()
{

    if (this->_canRead)
    {
        this->_canRead = false;
        this->_last_read = NTPInfo.getEpoch() < 1577836800 ? millis() : NTPInfo.getEpoch();
        if (this->_testOnly == false && this->_connected)
        {
            this->_temperature = this->readTemperature();
            this->_humidity = this->readHumidity();
            this->_pressure = bme.readPressure();
            Logging.log(LOG_VERBOSE, "Temperature Read is %0.3f%s",
                        EnvSensor.getTemperature(),
                        EnvSensor.getSymbol());
        }
        else
        {
            this->_temperature = 23.3;
            this->_humidity = 45.5;
            this->_pressure = 10856.0;
            Logging.log(LOG_VERBOSE, F("Testing Mode - Using Dummy Values!"));
        }
        this->_canRead = true;
        if (isnan(this->_temperature))
        {
            Logging.log(LOG_ERROR, F("No temperature read!!!!"));
            return false;
        }
        _count++;
    }
    return true;
}

const char *EnvSensorClass::getSymbol()
{
    return this->_symbol;
}

float EnvSensorClass::getTemperature()
{
    return this->_temperature;
}

// JsonObject EnvSensorClass::toJson()
// {
//     this->_doc["sensor_reads"] = _count;
//     return this->_doc.as<JsonObject>();
// }

void EnvSensorClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject("EnvSensor");
    json["temperature"] = this->_temperature;
    json["humidity"] = this->_humidity;
    json["pressure"] = this->_pressure;
    json["read_count"] = _count;
    json["read_session_time"] = this->_last_read;
}

void EnvSensorClass::pageInfo()
{
    ez.screen.clear();
    ez.header.show(F("Temp Info"));
    ez.buttons.show(F("#Back#Read"));
    ez.canvas.font(&FreeSans9pt7b);
    ez.canvas.lmargin(10);
    this->displayInfo();
}

void EnvSensorClass::displayInfo()
{
    const byte tab = 180;
    ez.canvas.clear();
    ez.canvas.pos(0, 10);
    ez.canvas.println("");
    ez.canvas.print(F("Temperature:"));
    ez.canvas.x(tab);
    ez.canvas.println(String(this->_temperature) + " " + this->_symbol);
    ez.canvas.print(F("Humidity:"));
    ez.canvas.x(tab);
    ez.canvas.println(String(this->_humidity) + F("%"));
    ez.canvas.print(F("Pressure:"));
    ez.canvas.x(tab);
    ez.canvas.println(String(this->_pressure));
    ez.canvas.print(F("Time:"));
    ez.canvas.x(tab);
    ez.canvas.println(String(NTPInfo.getFormattedTime()));
}

void EnvSensorClass::envInfo()
{
    this->pageInfo();
    ez.addEvent(updateInfo, 500);
    while (true)
    {
        String btn = ez.buttons.poll();
        if (btn == "Back")
        {
            Logging.log(LOG_VERBOSE, F("Exiting Sensor Page..."));
            ez.removeEvent(updateInfo);
            break;
        }
        if (btn == "Read")
        {
            Logging.log(LOG_VERBOSE, F("Manually Reading Sensor..."));
            this->read();
            this->pageInfo();
        }
    }
}

// Read the sensor data from the 1 Wire
byte EnvSensorClass::readDevice()
{
    Wire.beginTransmission(this->_id);
    Wire.write(0);
    if (Wire.endTransmission() != 0)
    {
        return 1;
    }
    Wire.requestFrom(this->_id, (uint8_t)5);

    for (int i = 0; i < 5; i++)
    {
        _datos[i] = Wire.read();
    };
    delay(50);
    if (Wire.available() != 0)
        return 2;
    if (_datos[4] != (_datos[0] + _datos[1] + _datos[2] + _datos[3]))
        return 3;
    return 0;
}

// convert the data read from sensor to temperature
float EnvSensorClass::readTemperature()
{
    float resultado = 0;
    byte error = this->readDevice();

    if (error != 0)
        return (float)error / 100;

    switch (this->_scale)
    {
    case ENV_CELSIUS:
        resultado = (_datos[2] + (float)_datos[3] / 10);
        break;
    case ENV_FAHRENHEIT:
        resultado = ((_datos[2] + (float)_datos[3] / 10) * 1.8 + 32);
        break;
    case ENV_KELVIN:
        resultado = (_datos[2] + (float)_datos[3] / 10) + 273.15;
        break;
    };
    return resultado;
}

// convert the data read from sensor to humidity
float EnvSensorClass::readHumidity()
{
    float resultado;
    byte error = this->readDevice();
    if (error != 0)
        return (float)error / 100;
    resultado = (_datos[0] + (float)_datos[1] / 10);
    return resultado;
}

EnvSensorClass EnvSensor;