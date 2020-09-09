#include "CloudInfo.h"
#include "LogInfo.h"
#include "AzureInstance.h"
#include "AwsInstance.h"

/**
 * Base Class Constructor
 * 
 * @param cloudType The cloudType name, so we load up the correct config
 */
CloudInfoClass::CloudInfoClass() : BaseConfigInfoClass("cloud")
{
    for (uint8_t i = 0; i < CERT_COUNT; i++)
    {
        this->_config.certificates[i].type = (CertType)i;
    }
}

/**
 * Begin the initialization of the cloud
 */
void CloudInfoClass::begin(SemaphoreHandle_t flag)
{
    this->_provider = NULL;    
    this->_config.semaphore = flag;
}

/**
 * overridden load JSON element into the cloud instance
 * 
 * @param json The ArduinoJson object that this element will be loaded from
 */
void CloudInfoClass::load(JsonObjectConst obj)
{
    LogInfo.log(LOG_VERBOSE, "Provider is %s", obj.containsKey("provider") ? obj["provider"].as<const char *>() : "UNKNOWN");
    this->_config.provider = CloudInfoClass::getProviderTypeFromString(obj.containsKey("provider") ? obj["provider"].as<const char *>() : "");

    if (obj.containsKey("certs"))
    {
        strcpy(this->_config.certificates[CT_CERT].fileName, obj["certs"].containsKey("certificate") ? obj["certs"]["certificate"].as<const char *>() : "");
        strcpy(this->_config.certificates[CT_KEY].fileName, obj["certs"].containsKey("key") ? obj["certs"]["key"].as<const char *>() : "");
        CloudInfoClass::loadCertificate(&this->_config.certificates[CT_CERT]);
        CloudInfoClass::loadCertificate(&this->_config.certificates[CT_KEY]);
    }
    if (obj.containsKey("iotHub"))
    {
        strcpy(this->_config.endPoint, obj["iotHub"].containsKey("endpoint") ? obj["iotHub"]["endpoint"].as<const char *>() : "");
        this->_config.port = obj["iotHub"].containsKey("port") ? obj["iotHub"]["port"].as<int>() : 8883;
        this->_config.sendTelemetry = obj["iotHub"].containsKey("sendTelemetry") ? obj["iotHub"]["sendTelemetry"].as<bool>() : false;
        this->_config.sendDeviceTwin = obj["iotHub"].containsKey("sendDeviceTwin") ? obj["iotHub"]["sendDeviceTwin"].as<bool>() : false;
        this->_config.sendInterval = obj["iotHub"].containsKey("intervalSeconds") ? obj["iotHub"]["intervalSeconds"].as<int>() : 60;
    }
    if (obj.containsKey("azure") && this->_config.provider == CPT_AZURE)
    {
        strcpy(this->_config.certificates[CT_CA].fileName, obj["azure"].containsKey("ca") ? obj["azure"]["ca"].as<const char *>() : "");
    }
    strcpy(this->ca_azure_fileName, obj["azure"].containsKey("ca") ? obj["azure"]["ca"].as<const char *>() : "");
    if (obj.containsKey("aws") && this->_config.provider == CPT_AWS)
    {
        strcpy(this->_config.certificates[CT_CA].fileName, obj["aws"].containsKey("ca") ? obj["aws"]["ca"].as<const char *>() : "");
    }
    strcpy(this->ca_aws_fileName, obj["aws"].containsKey("ca") ? obj["aws"]["ca"].as<const char *>() : "");

    CloudInfoClass::loadCertificate(&this->_config.certificates[CT_CA]);

    if (this->_config.provider == CPT_AZURE)
    {
        this->_provider = &Azure;
    }

    if (this->_config.provider == CPT_AWS)
    {
        this->_provider = &Aws;
    }

    LogInfo.log(LOG_VERBOSE, "Connect to %s [%s@%s:%i] Telemetry %s Interval %i seconds",
                CloudInfoClass::getStringFromProviderType(this->_config.provider),
                DeviceInfo.getDeviceId(),
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
    json["provider"] = CloudInfoClass::getStringFromProviderType(this->_config.provider);

    auto certs = json.createNestedObject("certs");
    certs["certificate"] = this->_config.certificates[CT_CERT].fileName;
    certs["key"] = this->_config.certificates[CT_KEY].fileName;

    auto iotHub = json.createNestedObject("iotHub");
    iotHub["endpoint"] = this->_config.endPoint;
    iotHub["port"] = this->_config.port;
    iotHub["sendTelemetry"] = this->_config.sendTelemetry;
    iotHub["sendDeviceTwin"] = this->_config.sendDeviceTwin;    
    iotHub["intervalSeconds"] = this->_config.sendInterval;

    auto azure_ca = json.createNestedObject("azure");
    azure_ca["ca"] = this->ca_azure_fileName;

    auto aws_ca = json.createNestedObject("aws");
    aws_ca["ca"] = this->ca_aws_fileName;
}

/**
 * overridden create a JSON element that will show the current cloud configuration
 * 
 * @param json The ArduinoJson object that this element will be added to.
 */
void CloudInfoClass::toJson(JsonObject ob)
{
    auto json = ob.createNestedObject(this->getSectionName());
    json["cloud"] = CloudInfoClass::getStringFromProviderType(this->_config.provider);
}

/**
 * Connect to the selected provider
 * 
 * @param builder This fuction pointer will build the data to be sent to the cloud
 * @param processor This function pointer will process the desired state
 * @return True if connected
 */
bool CloudInfoClass::connect(DATABUILDER builder, DESIREDPROCESSOR processor)
{
    if (this->getProvider() != NULL)
    {
        this->getProvider()->begin(builder, processor);
        return this->getProvider()->connect(&this->_config);
    }
    return false;
}

/**
 * Convert CloudProviderType to string
 * 
 * @param type The CloudProviderType
 */
const char *CloudInfoClass::getStringFromProviderType(CloudProviderType type)
{
    switch (type)
    {
    case CPT_AWS:
        return "aws";
    case CPT_AZURE:
        return "azure";
    default:
        return "default";
    }
    return "unknown";
}

/**
 * Convert string to CloudProviderType
 * 
 * @param type The string version of CloudProviderType
 */
CloudProviderType CloudInfoClass::getProviderTypeFromString(const char* type)
{
    if (Utilities::compare(type, "azure"))
    {
        return CPT_AZURE;
    }
    if (Utilities::compare(type, "aws"))
    {
        return CPT_AWS;
    } 
    return CPT_UNKNOWN;
}

/**
 * Load the certificate body into memory
 * 
 * @param cert The certificate struct to fill
 */
void CloudInfoClass::loadCertificate(CERTIFICATE *cert)
{
    if (strlen(cert->fileName) > 0)
    {
        size_t size = Utilities::fileSize(cert->fileName);
        if (size > 0)
        {
            LogInfo.log(LOG_VERBOSE, "File %s is %i bytes", cert->fileName, size);
            cert->contents = new char[size+1];
            Utilities::readFile(cert->fileName, cert->contents, size+1);
            LogInfo.log(LOG_VERBOSE, "Loaded Certificate (%s)", cert->fileName);
            if(heap_caps_check_integrity_all(true) == false)
            {
                LogInfo.log(LOG_ERROR, F("Heap Corruption detected! -Setup -0"));
            }              
        }
    }
}

/**
 * Check if there are any messages waiting at the broker for us
 */
void CloudInfoClass::tick()
{
    if( this->getProvider() != NULL)
    {
        this->getProvider()->sendData();
        delay(500);
        this->getProvider()->tick();
    }
}

/**
 * Get the cloud provider we are connecting to.
 * 
 * @return cloud provider instance to work with;
 */
BaseCloudProvider* CloudInfoClass::getProvider()
{
    return this->_provider;
}

CloudInfoClass CloudInfo;