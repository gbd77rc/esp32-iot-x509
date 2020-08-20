#include "CloudInfo.h"
#include "LogInfo.h"


// Internal WiFi Connection
WiFiClientSecure httpsClient;

/**
 * Base Class Constructor
 * 
 * @param cloudType The cloudType name, so we load up the correct config
 */
CloudInfoClass::CloudInfoClass(): BaseConfigInfoClass("cloud")
{
    this->_mqttClient = PubSubClient(httpsClient);
    for (uint8_t i = 0; i < CERT_COUNT;i++)
    {
        this->_config.certificates[i].type = (CertType)i;
    }    
}

/**
 * Get the cloud type name
 *  
 * @return The cloud type name
 */
char* CloudInfoClass::getCloudInstanceName()
{
    return this->_config.cloudInstance;
}

/**
 * Begin the initialization of the cloud
 */
void CloudInfoClass::begin()
{

}

/**
 * overridden load JSON element into the cloud instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void CloudInfoClass::load(JsonObjectConst obj)
{
    strcpy(this->_config.cloudInstance, obj.containsKey("connect") ? obj["connect"].as<const char*>() : "");
    if (obj.containsKey("certs"))
    {
        strcpy(this->_config.certificates[CT_CERT].fileName, obj["certs"].containsKey("certificate") ? obj["certs"]["certificate"].as<const char*>() : "");
        strcpy(this->_config.certificates[CT_KEY].fileName, obj["certs"].containsKey("key") ? obj["certs"]["key"].as<const char*>() : "");
    }
    if (obj.containsKey("iotHub"))
    {
        strcpy(this->_config.endPoint, obj["iotHub"].containsKey("endpoint") ? obj["iotHub"]["endpoint"].as<const char*>() : "");
        strcpy(this->_config.hubName, obj["iotHub"].containsKey("name") ? obj["iotHub"]["name"].as<const char*>() : "");
        this->_config.port = obj["iotHub"].containsKey("port") ? obj["iotHub"]["port"].as<int>() : 8883;
        this->_config.sendTelemetry = obj["iotHub"].containsKey("sendTelemetry") ? obj["iotHub"]["sendTelemetry"].as<bool>() : false;
        this->_config.sendInterval = obj["iotHub"].containsKey("intervalSeconds") ? obj["iotHub"]["intervalSeconds"].as<int>() : 60;
    }
    if( obj.containsKey("azure"))
    {
        strcpy(this->_config.certificates[CT_CA_AZURE].fileName, obj["azure"].containsKey("ca") ? obj["azure"]["ca"].as<const char*>() : "");
    }
    if( obj.containsKey("aws"))
    {
        strcpy(this->_config.certificates[CT_CA_AWS].fileName, obj["aws"].containsKey("ca") ? obj["aws"]["ca"].as<const char*>() : "");
    }

    LogInfo.log(LOG_VERBOSE, "Connect to %s [%s@%s:%i] Telementy %s Internval %i", 
                this->_config.cloudInstance,
                this->_config.hubName,
                this->_config.endPoint,
                this->_config.port,
                this->_config.sendTelemetry ? "Yes" : "No",
                this->_config.sendInterval);
}

/**
 * overridden save JSON element from the cloud instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void CloudInfoClass::save(JsonObject obj)
{
    auto json = obj.createNestedObject(this->_sectionName);
    json["connect"] = this->_config.cloudInstance;

    auto certs = json.createNestedObject("certs");
    certs["certficate"] = this->_config.certificates[CT_CERT].fileName;
    certs["key"] = this->_config.certificates[CT_KEY].fileName;

    auto iotHub = json.createNestedObject("iotHub");
    iotHub["endpoint"] = this->_config.endPoint;
    iotHub["name"] = this->_config.hubName;
    iotHub["port"] = this->_config.port;
    iotHub["sendTelemetry"] = this->_config.sendTelemetry;
    iotHub["intervalSeconds"] = this->_config.sendInterval;

    auto azure_ca = json.createNestedObject("azure");
    azure_ca["ca"] = this->_config.certificates[CT_CA_AZURE].fileName;

    auto aws_ca = json.createNestedObject("aws");
    aws_ca["ca"] = this->_config.certificates[CT_CA_AWS].fileName;
}

/**
 * overridden create a JSON element that will show the current cloud configuration
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */
void CloudInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject(this->getSectionName());
    json["cloud"] = this->_config.cloudInstance;
}

CloudInfoClass CloudInfo;