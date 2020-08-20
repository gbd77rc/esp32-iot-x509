#ifndef AZURECLOUD_H
#define AZURECLOUD_H

#include "DeviceInfo.h"
#include "CloudInfo.h"

class AzureInstanceClass : public BaseCloudProvider
{
public:
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    /**
     * Class Constructor
     */
    AzureInstanceClass() : BaseCloudProvider(CPT_AZURE)
    {
        strcpy(this->_twinDelta, "$iothub/twin/PATCH/properties");
        strcpy(this->_twinResponse, "$iothub/twin/res/#");
        strcpy(this->_deviceEvents, "devices/");
        strcat(this->_deviceEvents, DeviceInfo.getDeviceId());
        strcat(this->_deviceEvents, "/messages/events/");
        strcpy(this->_telemetry, "devices/");
        strcat(this->_telemetry, DeviceInfo.getDeviceId());
        strcat(this->_telemetry, "/messages/events/");        
    }

    bool connect(const IoTConfig *config) override;

private:
    void buildUserName(char *userName);
    bool getCurrentStatus();
};

extern AzureInstanceClass Azure;

#endif