# Cloud Instance

The main entry is the `CloudInfo` instance will be load the configuration and workout which cloud provider instance we should connect to.

Most of the actual sending is done in the `BaseCloudProvider` class.  The actual sending of data is generic for both Azure and AWS, the topic names need to change and that's about all.

The system will be set to QOS level 0, so we are not going to care about missing messages.

The `tick` function must be called regularly to make sure we have process waiting messages from the cloud MQTT broker.

## Example of use

    xSemaphore = xSemaphoreCreateMutex();
    CloudInfo.begin(xSemaphore);
    Configuration.begin("/config.json");
    Configuration.add(&CloudInfo);
    Configuration.load();


    CloudInfo.tick();
