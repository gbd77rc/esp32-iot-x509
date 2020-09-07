#include "BaseCloudProvider.h"
#include "LogInfo.h"
#include "DeviceInfo.h"
#include "WakeUpInfo.h"
#include "NTPInfo.h"
#include "LedInfo.h"

RTC_DATA_ATTR int _send_count;

/**
 * Static task function that is ran.  This will cast the parameters point to a struct 
 * and run the instance taskToRun overridden function
 * 
 * @param parameters The parameters to be passed to the task
 */
void BaseCloudProvider::checkTask(void *parameters)
{
    auto cloud = (struct cloudInstanceStruct *)parameters;
    LogInfo.log(LOG_VERBOSE, "Initializing %s Task and is connected %s",
                cloud->instance->getProviderType(),
                cloud->instance->getIsConnected() ? "Yes" : "No");
    vTaskSuspend(cloud->checkTaskHandle);
    for (;;)
    {
        WakeUp.suspendSleep();
        if (cloud->instance->getIsConnected())
        {
            if (xSemaphoreTake(cloud->instance->getSemaphore(), portMAX_DELAY))
            {
                cloud->instance->checkForMessages();
                xSemaphoreGive(cloud->instance->getSemaphore());
            }
            else
            {
                LogInfo.log(LOG_VERBOSE, "Could not get flag for %s", cloud->instance->getProviderType());
            }
        }
        vTaskDelay(20);
        WakeUp.resumeSleep();
        vTaskSuspend(cloud->checkTaskHandle);
    }
}

/**
 * Base Class Constructor
 * 
 * @param provider The cloud provider type
 */
BaseCloudProvider::BaseCloudProvider(CloudProviderType type)
{
    this->_providerType = type;
    this->_mqttClient = PubSubClient(this->_httpsClient);
    this->_cloudInstance.instance = this;
}

/**
 * Begin initialisation again
 * 
 * @param builder This is a pointer to a function that will build the json object the will be sent out
 */
void BaseCloudProvider::begin(DATABUILDER builder)
{
    this->_builder = builder;
    this->_cloudInstance.instance->loadTopics();
}

/**
* Is the instance connected
* 
* @return True if connected;
*/
bool BaseCloudProvider::getIsConnected()
{
    return this->_connected;
}

/**
 * Resume the check task
 */
void BaseCloudProvider::tick()
{
    vTaskResume(this->_cloudInstance.checkTaskHandle);
}

/**
 *  Check for the any waiting messages on the broker
 */
void BaseCloudProvider::checkForMessages()
{
    if (this->getIsConnected())
    {
        this->_mqttClient.loop();
    }
}

/**
 * Initialise the MQTT client and secure HTTP client
 */
void BaseCloudProvider::initialiseConnection(std::function<void(char *, uint8_t *, unsigned int)> callback)
{
    LedInfo.blinkOn(LED_CLOUD);
    this->_httpsClient.setCACert(this->_config->certificates[CT_CA].contents);
    this->_httpsClient.setCertificate(this->_config->certificates[CT_CERT].contents);
    this->_httpsClient.setPrivateKey(this->_config->certificates[CT_KEY].contents);
    this->_mqttClient.setServer(this->_config->endPoint, this->_config->port);
    this->_mqttClient.setCallback(callback);
}

/**
 * Initialise the MQTT Broker
 * 
 * @return True if successfully connected and subscribed
 */
bool BaseCloudProvider::mqttConnection()
{
    uint8_t retries = 0;
    LogInfo.log(LOG_VERBOSE, "Connecting to IoT Hub (%s):(%i) - (%s) @ %s",
                this->_config->endPoint,
                this->_config->port,
                DeviceInfo.getDeviceId(),
                NTPInfo.getISO8601Formatted().c_str());
    char userName[256];
    this->buildUserName(userName);
    uint32_t free = xPortGetFreeHeapSize();
    LogInfo.log(LOG_VERBOSE, "Current Free Heap Size is %i", free);
    while (!this->_mqttClient.connected() && retries < RECONNECT_RETRIES)
    {
        if (heap_caps_check_integrity_all(true) == false)
        {
            LogInfo.log(LOG_ERROR, F("Heap Corruption detected! -Base Mqtt Connect -1"));
            return false;
        }
        this->_mqttClient.setBufferSize(2048);
        if (this->_mqttClient.connect(DeviceInfo.getDeviceId(), userName, NULL))
        {
            if (heap_caps_check_integrity_all(true) == false)
            {
                LogInfo.log(LOG_ERROR, F("Heap Corruption detected! -Base Mqtt Connect -2"));
                return false;
            }
            LogInfo.log(LOG_VERBOSE, "Current Free Heap Size is %i", free);
            LogInfo.log(LOG_VERBOSE, "Connected Successfully to [%s]", this->_config->endPoint);
            LogInfo.log(LOG_VERBOSE, "Checking %i Topic for subscription", this->_topicsAdded);
            for (uint8_t i = 0; i < this->_topicsAdded; i++)
            {
                auto topic = &this->_topics[i];
                if (topic->type == TT_SUBSCRIBE)
                {
                    if (heap_caps_check_integrity(MALLOC_CAP_8BIT, true) == false)
                    {
                        LogInfo.log(LOG_ERROR, F("DRAM Heap Corruption detected! -Base Mqtt Connect -0"));
                        return false;
                    }
                    bool subbed = this->_mqttClient.subscribe(topic->topic, QOS_LEVEL);
                    LogInfo.log(LOG_VERBOSE, "Subscribed: %s (%s)",
                                subbed ? "Yes" : "No", topic->topic);
                    if (heap_caps_check_integrity(MALLOC_CAP_8BIT, true) == false)
                    {
                        LogInfo.log(LOG_ERROR, F("DRAM Heap Corruption detected! -Base Mqtt Connect -0"));
                        return false;
                    }
                    LogInfo.log(LOG_VERBOSE, "Current Free Heap Size is %i", free);
                }
            }
            break;
        }
        else
        {
            LogInfo.log(LOG_WARNING, "MQTT Connections State: %i", this->_mqttClient.state());
            delay(100);
            retries++;
        }
    }

    LedInfo.blinkOff(LED_CLOUD);
    this->_tryConnecting = false;
    if (!this->_mqttClient.connected())
    {
        LogInfo.log(LOG_WARNING, F("Timed out!"));
        return false;
    }
    this->_connected = true;
    if (this->getIsConnected())
    {
        LogInfo.log(LOG_VERBOSE, "Creating %s Check Messages Task on Core 0", this->getProviderType());
        xTaskCreatePinnedToCore(BaseCloudProvider::checkTask, "CheckMsgsTask",
                                16392,
                                (void *)&this->_cloudInstance,
                                1,
                                &this->_cloudInstance.checkTaskHandle,
                                0);
    }
    if (heap_caps_check_integrity_all(true) == false)
    {
        LogInfo.log(LOG_ERROR, F("Heap Corruption detected! -Base Mqtt Connect -2"));
        return false;
    }
    LedInfo.switchOn(LED_CLOUD);
    return this->getIsConnected();
}

/**
 * Get the provide type name string
 * 
 * @return The string pointer for the provider type name
 */
const char *BaseCloudProvider::getProviderType()
{
    switch (this->_providerType)
    {
    case CPT_AWS:
        return "aws";
    case CPT_AZURE:
        return "azure";
    case CPT_UNKNOWN:
        return "UNKNOWN";
    }
    return "";
}

/**
 * Get the semaphore flag
 * 
 * @return The semaphore flag
 */
const SemaphoreHandle_t BaseCloudProvider::getSemaphore()
{
    return this->_config->semaphore;
}

/**
 * Get the first topic matches the topic type.
 * 
 * @param type The topic type to search for
 * @return The topic found or an empty struct and type set to TT_UNKNOWN;
 */
const char *BaseCloudProvider::getFirstTopic(TopicType type)
{
    IoTTopic found = IoTTopic{"", TT_UNKNOWN, false};
    for (uint8_t i = 0; i < this->_topicsAdded; i++)
    {
        if (this->_topics[i].type == type)
        {
            found = this->_topics[i];
            break;
        }
    }

    if (found.type != TT_UNKNOWN)
    {
        if (found.appendUniqueId)
        {
            sprintf(this->_buffer, "%s%i", found.topic, _send_count);
        }
        else
        {
            strcpy(this->_buffer, found.topic);
        }
        return this->_buffer;
    }

    return "";
}

/**
 * Update the desired property to signal that we have accepted/rejected the change
 * 
 * @param property The name of the property to update
 * @param value The value of the property
 * @return True if successfully updated
 */
bool BaseCloudProvider::updateProperty(const char *property, JsonVariant value)
{
    bool sent = false;
    if (this->getIsConnected())
    {
        auto topic = this->getFirstTopic(TT_DEVICETWIN);
        _send_count++;

        DynamicJsonDocument doc(500);
        doc[property] = value;
        size_t len = measureJson(doc);
        char payload[len];
        serializeJson(doc, payload, len + 1);
        LogInfo.log(LOG_VERBOSE, "Updating Property to [%s]", topic);
        sent = this->_mqttClient.publish(
            topic,
            payload);
    }
    LogInfo.log(LOG_INFO, "Current Property status is %s at %s",
                sent ? "True" : "False", NTPInfo.getISO8601Formatted());

    return sent;
}

/**
 * Send data to device twin or 
 */
bool BaseCloudProvider::sendData()
{
    // Only send if we have valid Epoch (time greater then 2020-01-01)
    if (this->getIsConnected() && this->canSendNow() && NTPInfo.getEpoch() > 1577836800)
    {
        WakeUp.suspendSleep();
        LedInfo.blinkOn(LED_CLOUD);
        DynamicJsonDocument payload(800);
        auto root = payload.to<JsonObject>();
        this->_builder(root, true);
        this->sendDeviceReport(root);
        this->_builder(root, false);
        this->sendTelemetry(root);
        this->_lastSent = millis();
        LedInfo.blinkOff(LED_CLOUD);
        WakeUp.resumeSleep();
        return true;
    }
    return false;
}

/**
 * Send Device Twin Reported Properties
 * 
 * @param json The properties to be reported on
 * @return True if successfully sent
 */
bool BaseCloudProvider::sendDeviceReport(JsonObject json)
{
    LogInfo.log(LOG_VERBOSE, F("Calling Base sendDeviceReport"));
    bool sent = false;
    if (this->_config->sendDeviceTwin)
    {
        auto topic = this->getFirstTopic(TT_DEVICETWIN);
        _send_count++;
        size_t len = measureJson(json);
        char payload[len];
        serializeJson(json, payload, len + 1);
        LogInfo.log(LOG_VERBOSE, "Publishing to[%s]", topic);
        LogInfo.log(LOG_VERBOSE, F("Device Twin Payload"), json);
        LogInfo.log(LOG_INFO, "JSON Size : %u", measureJson(json));
        sent = this->_mqttClient.publish(
            topic,
            payload);

        LogInfo.log(LOG_INFO, "Current Publish status is %s at %s",
                    sent ? "True" : "False",
                    NTPInfo.getISO8601Formatted().c_str());
    }
    return sent;
}

/**
 * Send Telemetry Properties
 * 
 * @param json The properties to be reported on
 * @return True if successfully sent
 */
bool BaseCloudProvider::sendTelemetry(JsonObject json)
{
    bool sent = false;
    if (this->_config->sendTelemetry)
    {
        DynamicJsonDocument doc(500);
        doc.set(json);
        doc["location"] = DeviceInfo.getLocation();
        doc["deviceId"] = DeviceInfo.getDeviceId();
        doc["time_epoch"] = NTPInfo.getEpoch();
        size_t len = measureJson(doc);
        char payload[len];
        serializeJson(doc, payload, len + 1);
        auto topic = this->getFirstTopic(TT_TELEMETRY);
        _send_count++;
        LogInfo.log(LOG_VERBOSE, "Sending to[%s]", topic);
        LogInfo.log(LOG_VERBOSE, F("Telemetry MQTT Payload"), doc.as<JsonObject>());
        LogInfo.log(LOG_INFO, "JSON Size : %u", measureJson(doc));
        sent = this->_mqttClient.publish(
            topic,
            payload);

        LogInfo.log(LOG_INFO, "Current Send status is %s at %s",
                    sent ? "True" : "False", NTPInfo.getISO8601Formatted().c_str());
    }

    return sent;
}

/**
 * Can the telementry or reported be sent now
 * 
 * @return True if it can
 */
bool BaseCloudProvider::canSendNow()
{
    if ((millis() - this->_lastSent) > this->_config->sendInterval * ms_TO_S_FACTOR)
    {
        return true;
    }
    return false;
}