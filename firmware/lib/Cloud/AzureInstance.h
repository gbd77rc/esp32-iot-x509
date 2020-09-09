#ifndef AZURECLOUD_H
#define AZURECLOUD_H

#include "DeviceInfo.h"
#include "CloudInfo.h"

class AzureInstanceClass : public BaseCloudProvider
{
public:
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    AzureInstanceClass(); 
    bool connect(const IoTConfig *config) override;
    void processReply(char *topic, byte *payload, unsigned int length) override;
    bool updateProperty(JsonObjectConst element) override;      

protected:
    void buildUserName(char *userName) override;
    bool sendDeviceReport(JsonObject json) override;
    void loadTopics() override;

private:    
    bool getCurrentStatus();
};

extern AzureInstanceClass Azure;

#endif