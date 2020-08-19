#include "LogInfo.h"
#include "NTPInfo.h"
#include "AzureInfo.h"
#include "Utilities.h"
#include "DeviceInfo.h"
#include "WakeUpInfo.h"
#include "EnvSensor.h"
#include "GpsInfo.h"

RTC_DATA_ATTR int _Azure_count;

// Internal WiFi Connection
WiFiClientSecure httpsClient;

static AzureInfoClass *azureInfo;

TaskHandle_t AzureInfoClass::checkTaskHandle;
TaskHandle_t AzureInfoClass::sendDeviceTaskHandle;
TaskHandle_t AzureInfoClass::sendTelemetryTaskHandle;
SemaphoreHandle_t AzureInfoClass::semaphoreFlag;
bool AzureInfoClass::taskCreated;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    WakeUp.suspendSleep();
    azureInfo->desiredUpdate(topic, payload, length);

    WakeUp.resumeSleep();
}

void AzureInfoClass::checkTask(void *parameters)
{
    for (;;)
    {
        vTaskSuspend(AzureInfoClass::checkTaskHandle);
        if (xSemaphoreTake(AzureInfoClass::semaphoreFlag, portMAX_DELAY))
        {
            Logging.log(LOG_VERBOSE, "Resuming Check for Messages: Connected %s",
                        AzureInfo.isConnected() ? "Yes" : "No");
            if (AzureInfo.isConnected())
            {
                AzureInfo.checkForMessage();
            }

            if (Configuration.shouldSave())
            {
                Configuration.save();
            }
            xSemaphoreGive(AzureInfoClass::semaphoreFlag);
        }
    }
}

uint16_t AzureInfoClass::resumeCheckTask()
{
    if (AzureInfo.isConnected())
    {
        vTaskResume(AzureInfoClass::checkTaskHandle);
    }
    return 1000;
}

void AzureInfoClass::sendDeviceTask(void *parameters)
{
    for (;;)
    {
        vTaskSuspend(AzureInfoClass::sendDeviceTaskHandle);
        if (xSemaphoreTake(AzureInfoClass::semaphoreFlag, portMAX_DELAY))
        {
            Logging.log(LOG_VERBOSE, "Resuming Send Device Message: Connected %s:%s",
                        AzureInfo.isConnected() ? "Yes" : "No",
                        AzureInfo.canSend() ? "Yes" : "No");
            if (AzureInfo.isConnected() && AzureInfo.canSend())
            {
                DynamicJsonDocument payload(800);
                auto root = payload.to<JsonObject>();
                EnvSensor.toJson(root);
                GpsInfo.toJson(root);

                auto info = root.createNestedObject("BatteryInfo");
                info["state"] = ez.battery.getTransformedBatteryLevel();
                info["isCharging"] = M5.Power.isCharging();
                info["isFull"] = M5.Power.isChargeFull();
                // Only send if we have valid Epoch (time greater then 2020-01-01)
                if (NTPInfo.getEpoch() > 1577836800)
                {
                    AzureInfo.reportTwin(payload.as<JsonObject>());
                }
            }
            xSemaphoreGive(AzureInfoClass::semaphoreFlag);
        }
    }
}

uint16_t AzureInfoClass::resumeSendDeviceTask()
{
    if (AzureInfo.isConnected())
    {
        vTaskResume(AzureInfoClass::sendDeviceTaskHandle);
    }
    return 65000;
}

void AzureInfoClass::sendTelemetryTask(void *parameters)
{
    for (;;)
    {
        vTaskSuspend(AzureInfoClass::sendTelemetryTaskHandle);
        if (xSemaphoreTake(AzureInfoClass::semaphoreFlag, portMAX_DELAY))
        {
            Logging.log(LOG_VERBOSE, "Resuming Send Telemetry Message: Connected %s:%s",
                        AzureInfo.isConnected() ? "Yes" : "No",
                        AzureInfo.canSend() ? "Yes" : "No");
            if (AzureInfo.isConnected() && AzureInfo.canSend())
            {
                DynamicJsonDocument payload(800);
                auto root = payload.to<JsonObject>();
                EnvSensor.toJson(root);
                GpsInfo.toGeoJson(root);

                auto info = root.createNestedObject("BatteryInfo");
                info["state"] = ez.battery.getTransformedBatteryLevel();
                info["isCharging"] = M5.Power.isCharging();
                info["isFull"] = M5.Power.isChargeFull();
                // Only send if we have valid Epoch (time greater then 2020-01-01)
                if (NTPInfo.getEpoch() > 1577836800)
                {
                    AzureInfo.sendTelemetry(payload.as<JsonObject>());
                }
            }
            xSemaphoreGive(AzureInfoClass::semaphoreFlag);
        }
    }
}

uint16_t AzureInfoClass::resumeSendTelemetryTask()
{
    if (AzureInfo.isConnected())
    {
        vTaskResume(AzureInfoClass::sendTelemetryTaskHandle);
    }
    return 60000;
}

AzureInfoClass::AzureInfoClass() : BaseConfigInfoClass("azure"), _mqttClient(httpsClient)
{
}

bool AzureInfoClass::menu(ezMenu *callingMenu)
{
    AzureInfo.page1();
    while (true)
    {
        String btn = ez.buttons.poll();
        if (btn == "up1")
            AzureInfo.page1();
        if (btn == "Back")
            break;
    }
    return true;
}

void AzureInfoClass::page1()
{
    const byte tab = 240;
    ez.screen.clear();
    ez.header.show(F("IoT Info"));
    ez.buttons.show(F("#Back#"));
    ez.canvas.font(&FreeSans9pt7b);
    ez.canvas.lmargin(10);
    ez.canvas.println("");
    ez.canvas.print(F("Azure Connected:"));
    ez.canvas.x(tab);
    ez.canvas.println(this->_connected ? "Yes" : "No");
    ez.canvas.print(F("Azure Can Send Telemetry:"));
    ez.canvas.x(tab);
    ez.canvas.println(this->_canSend ? "Yes" : "No"); //   :)
    ez.canvas.print(F("Messages Sent:"));
    ez.canvas.x(tab);
    ez.canvas.println(_Azure_count);
}

void AzureInfoClass::begin()
{
    azureInfo = this;
    // Check if we are waking up or we have started because of manual reset or power on
    if (WakeUp.isPoweredOn())
    {
        _Azure_count = 0;
    }
    this->_certs[0] = CertificateInfo();
    this->_certs[1] = CertificateInfo();
    this->_certs[2] = CertificateInfo();
}

void AzureInfoClass::load(JsonObjectConst obj)
{
    strcpy(this->_hub_connection, obj.containsKey("hub_connection") ? obj["hub_connection"].as<const char *>() : "");
    strcpy(this->_hub_name, obj.containsKey("hub_name") ? obj["hub_name"].as<const char *>() : "");
    this->_port = obj["port"] ? obj["port"].as<uint16_t>() : 8883;
    strcpy(this->_certs[0].fileName, obj["certificate"] ? obj["certificate"].as<const char *>() : "");
    strcpy(this->_certs[1].fileName, obj["key"] ? obj["key"].as<const char *>() : "");
    strcpy(this->_certs[2].fileName, obj["ca"] ? obj["ca"].as<const char *>() : "");
    AzureInfoClass::loadCertificate(&this->_certs[0]);
    AzureInfoClass::loadCertificate(&this->_certs[1]);
    AzureInfoClass::loadCertificate(&this->_certs[2]);
    strcpy(this->_deviceTwin, "$iothub/twin/PATCH/properties");
    strcpy(this->_deviceEvents, "devices/");
    strcat(this->_deviceEvents, DeviceInfo.deviceId());
    strcat(this->_deviceEvents, "/messages/events/");
    strcpy(this->_telemetry, "devices/");
    strcat(this->_telemetry, DeviceInfo.deviceId());
    strcat(this->_telemetry, "/messages/events/");

    this->_canSend = obj["canSend"] ? obj["canSend"].as<bool>() : true;
    httpsClient.setCACert(this->_certs[2].contents);
    httpsClient.setCertificate(this->_certs[0].contents);
    httpsClient.setPrivateKey(this->_certs[1].contents);
    this->_mqttClient.setServer(this->_hub_name, this->_port);
    this->_mqttClient.setCallback(mqttCallback);
}

void AzureInfoClass::loadCertificate(CertificateInfo *cert)
{
    if (strlen(cert->fileName) > 0)
    {
        size_t size = Utilities::fileSize(cert->fileName);
        if (size > 0)
        {
            cert->contents = new char[size];
            Utilities::readFile(cert->fileName, cert->contents, size);
            Logging.log(LOG_VERBOSE, "Loaded Certificate (%s)", cert->fileName);
        }
    }
}

void AzureInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    //json["level"] = (int)this->_reportingLevel;
    json["hub_connection"] = this->_hub_connection;
    json["hub_name"] = this->_hub_name;
    json["certificate"] = this->_certs[0].fileName;
    json["key"] = this->_certs[1].fileName;
    json["ca"] = this->_certs[2].fileName;
    json["port"] = this->_port;
    json["canSend"] = this->_canSend;
}

bool AzureInfoClass::isConnected()
{
    return this->_connected;
}

bool AzureInfoClass::canSend()
{
    return this->_canSend;
}

void AzureInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject("AzureInfo");
    json["message_count"] = _Azure_count;
    json["last_sent"] = this->_last_sent;
}

void AzureInfoClass::buildUserName(char *userName)
{
    strcpy(userName, this->_hub_name);
    strcat(userName, "/");
    strcat(userName, DeviceInfo.deviceId());
    strcat(userName, "/?api-version=2018-06-30");
}

bool AzureInfoClass::connect()
{
    if (this->_tryConnecting == false)
    {
        this->_tryConnecting = true;

        uint8_t retries = 0;
        Logging.log(LOG_VERBOSE, "Connecting to IoT Hub (%s):(%i) - (%s)",
                    this->_hub_name, this->_port, DeviceInfo.deviceId());
        char userName[256];
        buildUserName(userName);
        while (!this->_mqttClient.connected() && retries < RECONNECT_RETRIES)
        {
            if (this->_mqttClient.connect(DeviceInfo.deviceId(), userName, NULL))
            {
                char topic[64];
                strcpy(topic, (String(this->_deviceTwin) + "/desired/#").c_str());
                Logging.log(LOG_VERBOSE, "Connected Successfully to [%s]", this->_hub_name);
                bool subbed = this->_mqttClient.subscribe(topic, QOS_LEVEL);
                Logging.log(LOG_VERBOSE, "Device Twin Subscribed: %s (%s)",
                            subbed ? "True" : "False", topic);
                subbed = this->_mqttClient.subscribe("$iothub/twin/res/#", QOS_LEVEL);
                Logging.log(LOG_VERBOSE, "Twin Response Topic Subscribed: %s (%s)",
                            subbed ? "True" : "False", "$iothub/twin/res/#");
                subbed = this->_mqttClient.subscribe(this->_deviceEvents, QOS_LEVEL);
                Logging.log(LOG_VERBOSE, "Device Events Topic Subscribed: %s (%s)",
                            subbed ? "True" : "False", this->_deviceEvents);
            }
            else
            {
                Logging.log(LOG_WARNING, "MQTT Connections State: %i", this->_mqttClient.state());
                delay(100);
                retries++;
            }
        }
        this->_tryConnecting = false;
        if (!this->_mqttClient.connected())
        {
            Logging.log(LOG_WARNING, F("Timed out!"));
            return false;
        }
        this->_connected = true;
        this->getCurrentStatus();
        if (AzureInfoClass::taskCreated == false && this->_connected)
        {
            Logging.log(LOG_VERBOSE, F("Creating Azure Tasks on Core 0"));
            xTaskCreatePinnedToCore(AzureInfoClass::sendDeviceTask, "SendIOTTask",
                                    10000, NULL, 3,
                                    &AzureInfoClass::sendDeviceTaskHandle,
                                    0);
            xTaskCreatePinnedToCore(AzureInfoClass::checkTask, "CheckForMessages",
                                    10000, NULL, 2,
                                    &AzureInfoClass::checkTaskHandle, 0);
            xTaskCreatePinnedToCore(AzureInfoClass::sendTelemetryTask, "SendTelemetry",
                                    10000, NULL, 3,
                                    &AzureInfoClass::sendTelemetryTaskHandle, 0);
            AzureInfoClass::taskCreated = true;
            ez.addEvent(AzureInfoClass::resumeSendTelemetryTask, 20000);
            ez.addEvent(AzureInfoClass::resumeSendDeviceTask, 10000);
            ez.addEvent(AzureInfoClass::resumeCheckTask, 1000);
        }
    }
    return this->_connected;
}

void AzureInfoClass::checkForMessage()
{
    this->_mqttClient.loop();
}

void AzureInfoClass::processDesiredStatus(JsonObject doc)
{
    if (doc.containsKey("desired"))
    {
        if (doc["desired"].containsKey("location"))
        {
            Logging.log(LOG_VERBOSE, F("Found Location Change"));
            if (DeviceInfo.setLocation(doc["desired"]["location"].as<char *>()))
            {
                this->updateProperty("location", doc["desired"]["location"]);
            }
        }
        if (doc["desired"].containsKey("canSend"))
        {
            Logging.log(LOG_VERBOSE, F("Found Send Status Change"));
            if (this->setCanSend(doc["desired"]["canSend"].as<bool>()))
            {
                this->updateProperty("canSend", doc["desired"]["canSend"]);
            }
        }
    }
}

void AzureInfoClass::processDesiredUpdate(JsonObject doc)
{
    if (doc.containsKey("location"))
    {
        Logging.log(LOG_VERBOSE, F("Found Location Change"));
        if (DeviceInfo.setLocation(doc["location"].as<char *>()))
        {
            this->updateProperty("location", doc["location"]);
        }
    }
    if (doc.containsKey("canSend"))
    {
        Logging.log(LOG_VERBOSE, F("Found Send Status Change"));
        if (this->setCanSend(doc["canSend"].as<bool>()))
        {
            this->updateProperty("canSend", doc["canSend"]);
        }
    }
}

void AzureInfoClass::desiredUpdate(char *topic, byte *payload, unsigned int length)
{
    Logging.log(LOG_VERBOSE, "Received Payload from [%s][%u]", topic, length);
    DynamicJsonDocument doc(length * 2);
    bool hasBody = false;
    if (length > 0)
    {
        DeserializationError err = deserializeJson(doc, (char *)payload);
        if (err)
        {
            Logging.log(LOG_ERROR, "Invalid payload: %i!!!!", err.code());
            return;
        }
        Logging.log(LOG_VERBOSE, F("MQTT Update Message"), doc.as<JsonObject>());
        hasBody = true;
    }
    // check for 204 on twin update
    char reply[64];
    strcpy(reply, "$iothub/twin/res/204/?$rid=");
    strcat(reply, String(_Azure_count).c_str());
    if (strstr(topic, reply) != NULL)
    {
        Logging.log(LOG_VERBOSE, "OK Reply from hub ");
    }
    strcpy(reply, "$iothub/twin/res/200/?$rid=");
    strcat(reply, String(_Azure_count).c_str());
    if (strstr(topic, reply) != NULL && hasBody)
    {
        this->processDesiredStatus(doc.as<JsonObject>());
    }
    //$iothub/twin/PATCH/properties/desired/?$version=9
    strcpy(reply, "$iothub/twin/PATCH/properties/desired/?$version=");
    if (strstr(topic, reply) != NULL && hasBody)
    {
        this->processDesiredUpdate(doc.as<JsonObject>());
    }

    Logging.log(LOG_VERBOSE, "Finished Updating - Body: %s",
                hasBody ? "Yes" : "No");
}

// {
//   "desired": {
//     "sending": true,
//     "$version": 8
//   },
//   "reported": {
//     "EnvSensor": {
//       "temperature": 23.9,
//       "humidity": 17.2,
//       "pressure": 101524.8,
//       "read_count": 6,
//       "read_session_time": 1594040459
//     },
//     "sending": true,
//     "$version": 81
//   }
// }

bool AzureInfoClass::setCanSend(bool enabled)
{
    if (enabled != this->_canSend)
    {
        Logging.log(LOG_VERBOSE, "Can Send Flag has changed! (%s)(%s)",
                    enabled ? "Yes" : "No",
                    this->_canSend ? "Yes" : "No");
        this->_canSend = enabled;
        this->_changed = true;
        return true;
    }
    return false;
}

bool AzureInfoClass::sendTelemetry(JsonObject json)
{
    bool sent = false;
    if (this->_connected and this->_canSend)
    {
        DynamicJsonDocument doc(500);
        doc.set(json);
        doc["location"] = DeviceInfo.location();
        doc["deviceId"] = DeviceInfo.deviceId();
        doc["time_epoch"] = NTPInfo.getEpoch();
        size_t len = measureJson(doc);
        char payload[len];
        serializeJson(doc, payload, len + 1);
        Logging.log(LOG_VERBOSE, "Sending to[%s]", this->_telemetry);
        Logging.log(LOG_VERBOSE, F("Telemetry MQTT Payload"), doc.as<JsonObject>());
        Logging.log(LOG_INFO, "JSON Size : %u", measureJson(doc));
        sent = this->_mqttClient.publish(
            this->_telemetry,
            payload);

        Logging.log(LOG_INFO, "Current Send status is %s at %s",
                    sent ? "True" : "False", NTPInfo.getISO8601Formatted());
    }
    else
    {
        Logging.log(LOG_INFO, "Sending is disabled?  %s",
                    this->_canSend ? "True" : "False", NTPInfo.getISO8601Formatted());
    }

    return sent;
}

bool AzureInfoClass::reportTwin(JsonObject json)
{
    bool sent = false;
    if (this->_connected and this->_canSend)
    {
        char topic[64];
        strcpy(topic, (String(this->_deviceTwin) + "/reported/?$rid=" + String(++_Azure_count)).c_str());
        DynamicJsonDocument doc(500);

        //JsonObject reported = doc.createNestedObject("reported");
        doc.set(json);
        //reported.set(json);
        doc["canSend"] = this->_canSend;
        doc["location"] = DeviceInfo.location();
        doc["deviceId"] = DeviceInfo.deviceId();
        size_t len = measureJson(doc);
        char payload[len];
        serializeJson(doc, payload, len + 1);
        Logging.log(LOG_VERBOSE, "Publishing to[%s]", topic);
        Logging.log(LOG_VERBOSE, F("Device Twin Payload"), doc.as<JsonObject>());
        Logging.log(LOG_INFO, "JSON Size : %u", measureJson(doc));
        sent = this->_mqttClient.publish(
            topic,
            payload);

        Logging.log(LOG_INFO, "Current Publish status is %s at %s",
                    sent ? "True" : "False", NTPInfo.getISO8601Formatted());
    }
    else
    {
        Logging.log(LOG_INFO, "Sending is disabled?  %s",
                    this->_canSend ? "True" : "False", NTPInfo.getISO8601Formatted());
    }

    return sent;
}

bool AzureInfoClass::updateProperty(const char *property, JsonVariant value)
{
    bool sent = false;
    if (this->_connected)
    {
        char topic[64];
        strcpy(topic, (String(this->_deviceTwin) + "/reported/?$rid=" + String(++_Azure_count)).c_str());
        DynamicJsonDocument doc(500);

        //JsonObject reported = doc.createNestedObject("reported");
        doc[property] = value;

        size_t len = measureJson(doc);
        char payload[len];
        serializeJson(doc, payload, len + 1);
        Logging.log(LOG_VERBOSE, "Updating Property to [%s]", topic);
        sent = this->_mqttClient.publish(
            topic,
            payload);
    }
    Logging.log(LOG_INFO, "Current Property status is %s at %s",
                sent ? "True" : "False", NTPInfo.getISO8601Formatted());

    return sent;
}

bool AzureInfoClass::getCurrentStatus()
{
    bool sent = false;
    if (this->_connected)
    {
        char topic[64];
        strcpy(topic, ("$iothub/twin/GET/?$rid=" + String(++_Azure_count)).c_str());
        Logging.log(LOG_VERBOSE, "Getting Current Status to[%s]", topic);
        sent = this->_mqttClient.publish(
            topic,
            nullptr);
    }
    Logging.log(LOG_INFO, "Current GET status is %s at %s", sent ? "True" : "False", NTPInfo.getISO8601Formatted());
    return sent;
}

AzureInfoClass AzureInfo;