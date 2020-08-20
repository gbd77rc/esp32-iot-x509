#include "BaseCloudProvider.h"

/**
 * Base Class Constructor
 * 
 * @param provider The cloud provider type
 */
BaseCloudProvider::BaseCloudProvider(CloudProviderType type)
{
    this->_providerType = type;
    this->_mqttClient = PubSubClient(this->_httpsClient);
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
 * Initialise the MQTT client and secure HTTP client
 */
void BaseCloudProvider::initialiseConnection(std::function<void (char *, uint8_t *, unsigned int)> callback)
{
    this->_httpsClient.setCACert(this->_config->certificates[CT_CA].contents);
    this->_httpsClient.setCertificate(this->_config->certificates[CT_CERT].contents);
    this->_httpsClient.setPrivateKey(this->_config->certificates[CT_KEY].contents);
    this->_mqttClient.setServer(this->_config->endPoint, this->_config->port);
    this->_mqttClient.setCallback(callback);    
}