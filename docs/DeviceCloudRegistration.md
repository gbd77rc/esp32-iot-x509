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

## AWS Registration

Now AWS has a concept of [Thing types](https://docs.aws.amazon.com/iot/latest/developerguide/thing-types.html).  This allows you to group things together and store common configuration associated with all things aka devices.  This makes management of the things easier.  As this is a simple system for now we want worry about it as it is optional.

During the writing of these articles AWS released a new feature on IoT Core Shadow.  This is called `Named Shadow`, basically it allow multiple Shadows to be assigned to a device.  It is explained in this [article](https://aws.amazon.com/about-aws/whats-new/2020/07/aws-iot-core-now-supports-multiple-shadows-for-a-single-iot-device/).  We still use the what is called the `classic` shadow.  I will create an additional article later in the year to explain how to use this new feature.

### Policy Creation

All devices are associated with a policy before AWS can accept messages from them.  Policy's can be assigned individually to a device or a IoT Group and the device assigned to that group.  Now the policy we are going to create a not that restrictive.  I will not cover how to use policy/permissions, that can be done via AWS own [documentation](https://docs.aws.amazon.com/iot/latest/developerguide/iot-policies.html).

With that being said, the following policy will be used.  This is generic one in the sense it will allow all devices to connect and use the shadow service.  You should review it for production.

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Action": [
        "iot:Connect"
      ],
      "Effect": "Allow",
      "Resource": "*"
    },
    {
      "Action": [
        "iot:Subscribe"
      ],
      "Effect": "Allow",
      "Resource": "*"
    },  
    {
      "Action": [
        "iot:Publish",
        "iot:Receive"
      ],
      "Effect": "Allow",
      "Resource": "*"
    },  
    {
      "Action": [
        "iot:DeleteThingShadow",
        "iot:GetThingShadow",
        "iot:UpdateThingShadow"
      ],
      "Effect": "Allow",
      "Resource": "*"
    }
  ]
}
```

This policy will allow the device to connect, create, update and delete its own shadow and publish/subscribe to any topic.

Create a text file called `ot-iot-policy.json` and copy the above policy json into it and save.  

Using the following command you should be able to create the policy.

```shell
▶ aws iot create-policy --policy-name ot-iot-device-policy --policy-document file://ot-iot-policy.json
```

It should give you the following output

```json
{
    "policyName": "ot-iot-device-policy",
    "policyArn": "arn:aws:iot:eu-west-2::policy/ot-iot-device-policy",
    "policyDocument": "{\n  \"Version\": \"2012-10-17\",\n  \"Statement\": [\n    {\n      \"Action\": [\n        \"iot:Connect\"\n      ],\n      \"Effect\": \"Allow\",\n      \"Resource\": \"*\"\n    },\n    {\n      \"Action\": [\n        \"iot:Subscribe\"\n      ],\n      \"Effect\": \"Allow\",\n      \"Resource\": \"*\"\n    },  \n    {\n      \"Action\": [\n        \"iot:Publish\",\n        \"iot:Receive\"\n      ],\n      \"Effect\": \"Allow\",\n      \"Resource\": \"*\"\n    },  \n    {\n      \"Action\": [\n        \"iot:DeleteThingShadow\",\n        \"iot:GetThingShadow\",\n        \"iot:UpdateThingShadow\"\n      ],\n      \"Effect\": \"Allow\",\n      \"Resource\": \"*\"\n    }\n  ]\n}\n\n",
    "policyVersionId": "1"
}
```

> Now make sure you use the IoT policy create command and not the IAM one, or you will spend an hour trying to workout way you can't attach it to the group!

### IoT Group

Now we are ready to create the thing group and attach to the policy to it.

```shell
▶ aws iot create-thing-group --thing-group-name ot-devices  
```

It should give an output of

```json
{
    "thingGroupName": "ot-devices",
    "thingGroupArn": "arn:aws:iot:eu-west-2:<account id>:thinggroup/ot-devices",
    "thingGroupId": "ae...f53"
}
```

Time to attach the policy to it.  Just make sure you update the `account id`!

```shell
▶ aws iot attach-policy --target "arn:aws:iot:eu-west-2:<account id>:thinggroup/ot-devices" --policy-name "ot-iot-device-policy"
```

The is no real output here.

### Registration

The following command will create and active the device on Aws.  It follows the [Single-thing provision](https://docs.aws.amazon.com/iot/latest/developerguide/provision-w-cert.html) concept.

The first step to make this easier is to register the device certificate.  This is not required if you use the JITP process, but as we are using single use it make it easier in the long run.  

Using the files created in [Device Certificate Creation](./docs/DeviceCertificate.md) run the following.

```shell
▶ aws iot register-certificate --certificate-pem file://105783B5AA8C-cert.pem --ca-certificate-pem file://dev-root-ca.pem --set-as-active
```

The output should be something like this.

```json
{
    "certificateArn": "arn:aws:iot:eu-west-2::cert/73d9790dd8cfa1dc304a154cb41c15147852ef14d2ec3d95a94f59df268ae2e1",
    "certificateId": "73d9790dd8cfa1dc304a154cb41c15147852ef14d2ec3d95a94f59df268ae2e1"
}
```

> Now if you get Invalid Certificate, then the possible answer is you created certificate without the -sha256 flag.
> To check run `q`, you should see `Signature Algorithm: sha256WithRSAEncryption` in the output somewhere, if not regenerate the cert again.

Make a note of the `certificateId`, you will need it.

Provisioning template for single use is required.  If JITP is used then you could have assigned this template the the CA certificate.  The following template will be used.

```json
{
    "Parameters" : {
        "DeviceId" : {
            "Type" : "String"
        },
        "CertificateId" : {
            "Type" : "String"
        }
    },
    "Resources" : {
        "thing" : {
            "Type" : "AWS::IoT::Thing",
            "OverrideSettings":{
                "AttributePayload":"MERGE",
                "ThingGroups":"DO_NOTHING"
            },
            "Properties" : {
                "AttributePayload" : { "version" : "v1"},
                "ThingGroups" : ["ot-devices"],
                "ThingName" : { "Ref" : "DeviceId"}
            }
        },
        "certificate" : {
            "Type" : "AWS::IoT::Certificate",
            "Properties" : {
                "CertificateId": {"Ref" : "CertificateId"}
            }
        }
    }
}
```

Now using that template we can create the following command

```shell
▶ aws iot register-thing \
    --template-body '{"Parameters":{"DeviceId":{"Type":"String"},"CertificateId":{"Type":"String"}},"Resources":{"thing":{"Type":"AWS::IoT::Thing","OverrideSettings":{"AttributePayload":"MERGE","ThingGroups":"DO_NOTHING"},"Properties":{"AttributePayload":{"version":"v1"},"ThingGroups":["ot-devices"],"ThingName":{"Ref":"DeviceId"}}},"certificate":{"Type":"AWS::IoT::Certificate","Properties":{"CertificateId":{"Ref":"CertificateId"}}}}}' \
    --parameters '{"DeviceId":"OT-105783B5AA8C","CertificateId":"73d9790dd8cfa1dc304a154cb41c15147852ef14d2ec3d95a94f59df268ae2e1"}'
```

This should give you an output of

```json
{
    "certificatePem": "-----BEGIN CERTIFICATE-----\n...\n-----END CERTIFICATE-----\n",
    "resourceArns": {
        "certificate": "arn:aws:iot:eu-west-2::cert/73d9790dd8cfa1dc304a154cb41c15147852ef14d2ec3d95a94f59df268ae2e1",
        "thing": "arn:aws:iot:eu-west-2::thing/OT-105783B5AA8C"
    }
}
```

Now we need the `endpoint` for the IoT Core, this can be returned using this command.

```shell
▶ aws iot describe-endpoint --endpoint-type "iot:Data-ATS"
```

The output will be like

```json
{
    "endpointAddress": "a...9-ats..iot.eu-west-2.amazonaws.com"
}
```

To test AWS, update the `provider` property to `aws`.  We will need to update the `endpoint` to the `endpointAddress`.  Then you can run the `Upload File System image` command to upload to the device.  This is explained in the [Device Development](./FirmwareDevelopment.md) article.

