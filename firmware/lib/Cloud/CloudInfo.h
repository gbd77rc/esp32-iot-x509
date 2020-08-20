#ifndef BASECLOUD_H
#define BASECLOUD_H

#include "Config.h"
#include "Utilities.h"
#include "CloudMisc.h"
#include "BaseCloudProvider.h"

class CloudInfoClass : public BaseConfigInfoClass
{
public:
    CloudInfoClass();

    void begin();
    bool connect();
    void load(JsonObjectConst obj) override;
    void save(JsonObject ob) override;
    void toJson(JsonObject ob) override;

private:
    static const char* getStringFromProviderType(CloudProviderType type);
    static CloudProviderType getProviderTypeFromString(const char* type);    
    static void loadCertificate(CERTIFICATE *cert);

    IOTCONFIG _config;
    BaseCloudProvider *_provider;
    // Need this so we can save the JSON correctly
    char ca_azure_fileName[32];
    char ca_aws_fileName[32];
};

extern CloudInfoClass CloudInfo;

// "endpoint": "",
// "name": "",
// "port": 8883,
// "sendTelemetry": true,
// "intervalSeconds": 45

#endif