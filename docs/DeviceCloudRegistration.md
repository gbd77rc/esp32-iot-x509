# Device Registration In The Cloud

By now you should have installed the command line tools for both Azure and AWS.  I will not cover that here.

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