#include "AzureInstance.h"
#include "LogInfo.h"
#include "NTPInfo.h"

RTC_DATA_ATTR int _Azure_count;

/**
 * This the static callback for processing messages return from the IoT broker
*/
void AzureInstanceClass::mqttCallback(char *topic, byte *payload, unsigned int length)
{
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
        this->_config = config;
        this->initialiseConnection(AzureInstanceClass::mqttCallback);
 
        this->_tryConnecting = true;

        uint8_t retries = 0;
        LogInfo.log(LOG_VERBOSE, "Connecting to IoT Hub (%s):(%i) - (%s)",
                    this->_config->hubName,
                    this->_config->port,
                    DeviceInfo.getDeviceId());
        char userName[256];
        buildUserName(userName);
        while (!this->_mqttClient.connected() && retries < RECONNECT_RETRIES)
        {
            if (this->_mqttClient.connect(DeviceInfo.getDeviceId(), userName, NULL))
            {
                char topic[64];
                strcpy(topic, (String(this->_twinDelta) + "/desired/#").c_str());
                LogInfo.log(LOG_VERBOSE, "Connected Successfully to [%s]", this->_config->hubName);
                bool subbed = this->_mqttClient.subscribe(topic, QOS_LEVEL);
                LogInfo.log(LOG_VERBOSE, "Device Twin Subscribed: %s (%s)",
                            subbed ? "Yes" : "No", topic);
                subbed = this->_mqttClient.subscribe(this->_twinResponse, QOS_LEVEL);
                LogInfo.log(LOG_VERBOSE, "Twin Response Topic Subscribed: %s (%s)",
                            subbed ? "True" : "False", this->_twinResponse);
                subbed = this->_mqttClient.subscribe(this->_deviceEvents, QOS_LEVEL);
                LogInfo.log(LOG_VERBOSE, "Device Events Topic Subscribed: %s (%s)",
                            subbed ? "True" : "False", this->_deviceEvents);
            }
            else
            {
                LogInfo.log(LOG_WARNING, "MQTT Connections State: %i", this->_mqttClient.state());
                delay(100);
                retries++;
            }
        }
        this->_tryConnecting = false;
        if (!this->_mqttClient.connected())
        {
            LogInfo.log(LOG_WARNING, F("Timed out!"));
            return false;
        }
        this->_connected = true;
        this->getCurrentStatus();
        // if (AzureInfoClass::taskCreated == false && this->_connected)
        // {
        //     Logging.log(LOG_VERBOSE, F("Creating Azure Tasks on Core 0"));
        //     xTaskCreatePinnedToCore(AzureInfoClass::sendDeviceTask, "SendIOTTask",
        //                             10000, NULL, 3,
        //                             &AzureInfoClass::sendDeviceTaskHandle,
        //                             0);
        //     xTaskCreatePinnedToCore(AzureInfoClass::checkTask, "CheckForMessages",
        //                             10000, NULL, 2,
        //                             &AzureInfoClass::checkTaskHandle, 0);
        //     xTaskCreatePinnedToCore(AzureInfoClass::sendTelemetryTask, "SendTelemetry",
        //                             10000, NULL, 3,
        //                             &AzureInfoClass::sendTelemetryTaskHandle, 0);
        //     AzureInfoClass::taskCreated = true;
        // }
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
    LogInfo.log(LOG_INFO, "Current GET status is %s at %s", sent ? "True" : "False", NTPInfo.getISO8601Formatted());
    return sent;
}

/**
 * Build the user name to connect to the hub
 */
void AzureInstanceClass::buildUserName(char *userName)
{
    strcpy(userName, this->_config->hubName);
    strcat(userName, "/");
    strcat(userName, DeviceInfo.getDeviceId());
    strcat(userName, "/?api-version=2018-06-30");
}

AzureInstanceClass Azure;