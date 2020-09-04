#ifndef AWSCLOUD_H
#define AWSCLOUD_H

#include "DeviceInfo.h"
#include "CloudInfo.h"

class AwsInstanceClass : public BaseCloudProvider
{
public:
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    AwsInstanceClass(); 
    bool connect(const IoTConfig *config) override;
    void processReply(char *topic, byte *payload, unsigned int length) override;

protected:
    void buildUserName(char *userName) override;
    void processDesiredStatus(JsonObject doc) override;
    void loadTopics() override;

private:    
    bool getCurrentStatus();
    char _shadowPrefix[64];
};

extern AwsInstanceClass Aws;

#endif