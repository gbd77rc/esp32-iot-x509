#include "AzureInstance.h"
#include "LogInfo.h"
#include "NTPInfo.h"
#include "WakeUpInfo.h"
#include "LedInfo.h"

RTC_DATA_ATTR int _Azure_count;

/**
 * This the static callback for processing messages return from the IoT broker
*/
void AzureInstanceClass::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    WakeUp.suspendSleep();
    LedInfo.blinkOn(LED_CLOUD);
    Azure.processReply(topic, payload, length);
    LedInfo.blinkOff(LED_CLOUD);
    WakeUp.resumeSleep();
}

/**
 * Azure Instance Constructor
 */
AzureInstanceClass::AzureInstanceClass() : BaseCloudProvider(CPT_AZURE)
{
}

/**
 * Send Device Twin Reported Properties
 * 
 * @param json The properties to be reported on
 * @return True if successfully sent
 */
bool AzureInstanceClass::sendDeviceReport(JsonObject json)
{
    LogInfo.log(LOG_VERBOSE, F("Calling Azure sendDeviceReport"));
    DynamicJsonDocument doc(500);
    doc.set(json);
    DeviceInfo.toJson(doc.as<JsonObject>());
    doc["time_epoch"] = NTPInfo.getEpoch();    
    return BaseCloudProvider::sendDeviceReport(doc.as<JsonObject>());
}

/**
 * Connect and initialise the broker.
 * 
 * @return True if connected
 */
bool AzureInstanceClass::connect(const IoTConfig *config)
{
    if (this->_tryConnecting == false)
    {
        this->_tryConnecting = true;
        this->_config = config;
        this->initialiseConnection(AzureInstanceClass::mqttCallback);
        if (this->mqttConnection())
        {
            this->getCurrentStatus();
        }
    }
    return this->getIsConnected();
}

/**
 * Get the current twin status of the device
 * 
 * @return The current status of the device twin
 */
bool AzureInstanceClass::getCurrentStatus()
{
    bool sent = false;
    if (this->getIsConnected())
    {
        char topic[64];
        strcpy(topic, ("$iothub/twin/GET/?$rid=" + String(++_Azure_count)).c_str());
        LogInfo.log(LOG_VERBOSE, "Getting Current Status to[%s]", topic);
        sent = this->_mqttClient.publish(
            topic,
            nullptr);
    }
    LogInfo.log(LOG_INFO, "Current GET status is %s at %s", sent ? "True" : "False", NTPInfo.getISO8601Formatted().c_str());
    return sent;
}

/**
 * Build the user name to connect to the hub
 */
void AzureInstanceClass::buildUserName(char *userName)
{
    strcpy(userName, this->_config->endPoint);
    strcat(userName, "/");
    strcat(userName, DeviceInfo.getDeviceId());
    strcat(userName, "/?api-version=2018-06-30");
}

/**
 * Check the reply received from the MQTT broker
 * 
 * @param topic The topic the message was received from 
 * @param payload The body of the message, generally in JSON format
 * @param length The size of the payload
 */
void AzureInstanceClass::processReply(char *topic, byte *payload, unsigned int length)
{
    LogInfo.log(LOG_VERBOSE, "Received Azure Reply from [%s][%u]", topic, length);
    DynamicJsonDocument doc(length * 2);
    bool hasBody = false;
    if (length > 0)
    {
        DeserializationError err = deserializeJson(doc, (char *)payload);
        if (err)
        {
            LogInfo.log(LOG_ERROR, "Invalid payload: %i!!!!", err.code());
            return;
        }
        LogInfo.log(LOG_VERBOSE, F("MQTT Update Message"), doc.as<JsonObject>());
        hasBody = true;
    }
    // check for 204 on twin update
    char reply[64];
    strcpy(reply, "$iothub/twin/res/204/?$rid=");
    strcat(reply, String(_Azure_count).c_str());
    if (strstr(topic, reply) != NULL)
    {
        LogInfo.log(LOG_VERBOSE, "OK Reply from hub ");
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
        this->processDesiredStatus(doc.as<JsonObject>());
    }

    LogInfo.log(LOG_VERBOSE, "Finished Updating - Body: %s",
                hasBody ? "Yes" : "No");
}

/**
 * Process the desired properties and set the configuration elements
 * 
 * @param doc The doc object that contains the desired element.
 */
void AzureInstanceClass::processDesiredStatus(JsonObject doc)
{
    JsonObject element;
    if (doc.containsKey("desired"))
    {
        element = doc["desired"].as<JsonObject>();
    }
    else 
    {
        element = doc;
    }

    if (element.containsKey("location"))
    {
        LogInfo.log(LOG_VERBOSE, F("Found Location Change"));
        if (DeviceInfo.setLocation(element["location"].as<char *>()))
        {
            this->updateProperty("location", element["location"]);
        }
    }    
}

/**
 * Load the topics into the provider
 */
void AzureInstanceClass::loadTopics()
{
    strcpy(this->_topics[0].topic, "$iothub/twin/PATCH/properties/desired/#");
    this->_topics[0].type = TT_SUBSCRIBE;
    strcpy(this->_topics[1].topic, "$iothub/twin/res/#");
    this->_topics[1].type = TT_SUBSCRIBE;
    strcpy(this->_topics[2].topic, "devices/");
    strcat(this->_topics[2].topic, DeviceInfo.getDeviceId());
    strcat(this->_topics[2].topic, "/messages/events/");
    this->_topics[2].type = TT_SUBSCRIBE;
    strcpy(this->_topics[3].topic, "devices/");
    strcat(this->_topics[3].topic, DeviceInfo.getDeviceId());
    strcat(this->_topics[3].topic, "/messages/events/");
    this->_topics[3].type = TT_TELEMETRY;
    strcpy(this->_topics[4].topic, "$iothub/twin/PATCH/properties/reported/?$rid=");
    this->_topics[4].type = TT_DEVICETWIN;
    this->_topics[4].appendUniqueId = true;
    strcpy(this->_topics[5].topic, "$iothub/twin/GET/?$rid=");
    this->_topics[5].type = TT_SYNCDEVICETWIN;
    this->_topics[5].appendUniqueId = true;
    this->_topicsAdded = 6;
}

AzureInstanceClass Azure;