# Device Registration In The Cloud

By now you should have installed the command line tools for both Azure and AWS.  I will not cover that here.  From the `config.json` the following element can be found.

```json
    "cloud": {
        "provider": "azure",
        "certs": {
            "certificate": "/cloud/device-cert.pem",
            "key": "/cloud/device.key"
        },
        "iotHub": {
            "endpoint": "dev-ot-iot-hub.azure-devices.net",
            "port": 8883,
            "sendTelemetry": true,
            "sendDeviceTwin": true,
            "intervalSeconds": 45
        },
        "azure": {
            "ca": "/cloud/portal-azure-com.pem"
        },
        "aws": {
            "ca": "/cloud/console-aws-com.pem"
        }
    }
```

We will test Azure first, as you can see I have default the `provider` property to this value.  We will need to update the `endpoint` to the `hostname` that was create in [DeviceCertificate.md](./DeviceCertificate.md) Azure IoT Hub creation step.  When the certificates have been copied and `endpoint` updated you can run the `Upload File System image` command to upload to the device.  This is explained in the [Device Development](./FirmwareDevelopment.md) article.

## Azure Registration

The following command will create and active the device on Azure.

```shell
▶ az iot hub device-identity create --hub-name dev-ot-iot-hub --device-id OT-105783B5AA8C --am x509_ca
{
  "authentication": {
    "symmetricKey": {
      "primaryKey": null,
      "secondaryKey": null
    },
    "type": "certificateAuthority",
    "x509Thumbprint": {
      "primaryThumbprint": null,
      "secondaryThumbprint": null
    }
  },
  "capabilities": {
    "iotEdge": false
  },
  "cloudToDeviceMessageCount": 0,
  "connectionState": "Disconnected",
  "connectionStateUpdatedTime": "0001-01-01T00:00:00",
  "deviceId": "OT-105783B5AA8C",
  "deviceScope": null,
  "etag": "MTcwNTI0MDQx",
  "generationId": "637347212881300554",
  "lastActivityTime": "0001-01-01T00:00:00",
  "parentScopes": [],
  "status": "enabled",
  "statusReason": null,
  "statusUpdatedTime": "0001-01-01T00:00:00"
}
```

The important parts here are the device id, which **MUST** match the `CN` in the certificate on the device and its `status` is enabled.  There is an issue with Azure and asymmetric keys aka x509, and having the `iotEdge` flag enabled.  It is currently not supported.  I have not tried OTA updates on an Azure IoT device yet, so don't know if this would cause it to fail or not.  I believe this flag is for the [Azure IoT Edge](https://azure.microsoft.com/en-us/services/iot-edge/) service.  More investigation is required.

Now if you follow the instructions in [Device Development](./FirmwareDevelopment.md) article, and upload the program to the device and connect to WiFi router.  After a while the device will go to sleep, by then it should have sent a few device twin messages and telemetry messages successfully to Azure.

This command will show the current device twin state.

```shell
▶ az iot hub device-twin show --device-id OT-105783B5AA8C --hub-name dev-ot-iot-hub
```

I have not shown the output as it is big.  If you don't see anything in the `desired` element then there is something wrong.  Check the logs on the device, as explained in the [Device Development](./FirmwareDevelopment.md) article.
