# Device Certificate

Each device should have its own certificate with its `Common Name` set to the device identifier.  

## Device Id aka Common Name

So the first step is to work out the device identifier.  If we the manufacturer, then this would be part of the automatic certificate creation process when the device is produced.  As we are not then the quick why is to flash the following code to it. See the [Device Development](./FirmwareDevelopment.md) article on possible ways of doing this.

```c++
#include <Arduino.h>
#include "LogInfo.h"
#include "DisplayInfo.h"

void setup()
{
    Serial.begin(115200);
    LogInfo.begin();
    OledDisplay.begin();  
    LogInfo.log(LOG_VERBOSE, "CPU Id is %s", LogInfo.getUniqueId());
    OledDisplay.displayLine(0, 10, "ID : %s", LogInfo.getUniqueId());
}

void loop(){}
```

Once flashed the OLED display will show the CPU Id, we combine this with the `prefix` found in this element in the `config.json` file.

```json
"device": {
  "prefix": "OT",
  "wakeup": 1200,
  "sleep": 30,
  "location": "<UNKNOWN>"
}
```

So if the CPU Id is `AC8684B5AA8C` then the actual device Id will be `OT-AC8684B5AA8C`.

## From Root Certificate to Device Certificate

We need a private CA to generate the device certificate and create validation chain.  Normally you would have a signing certificate generated from a recognized CA like [digicert](https://www.digicert.com/).  

> I am not associated them, or have ever brought certificates from them.

As we don't want the expense, we will create our own and upload that  the cloud, which they allow.  To this make sure you have an `openssl` application installed on your os.  I use a Mac OSX here, but it is available for Windows and Linux.  The command line may differ on Windows.  You will need to do some research on the command if it fails.

The `file names` and `CA Common Name` can be changed to suit your needs.  Just make it consistent.

### Mac OSX OpenSSL Workaround

If you are using a Mac OSX, then it is more then likely has LibraSSL installed.  It does not have the `-addext` option to the command.  Which is need to add the `basicConstraints` properties to the root certificate.    To get around this do the following.

```shell
▶ nano rootca.conf
```

Add this to the file.

```shell

[ req ]
distinguished_name       = req_distinguished_name
extensions               = v3_ca
req_extensions           = v3_ca

[ v3_ca ]
basicConstraints         = CA:TRUE

[ req_distinguished_name ]
countryName                     = Country Name (2 letter code)
countryName_min                 = 2
countryName_max                 = 2
stateOrProvinceName             = State or Province Name (full name)
localityName                    = Locality Name (eg, city)
0.organizationName              = Organization Name (eg, company)
organizationalUnitName          = Organizational Unit Name (eg, section)
commonName                      = Common Name (eg, fully qualified host name)
commonName_max                  = 64
emailAddress                    = Email Address
emailAddress_max                = 6
```

If you following [https://github.com/jetstack/cert-manager/issues/279](https://github.com/jetstack/cert-manager/issues/279) this github issue, it will expand other ways around this as well.

### Create the CA Key

```shell
▶ openssl genrsa -out dev-root-ca.key 4096
Generating RSA private key, 4096 bit long modulus
...............................................................................................++
...........................................++
e is 65537 (0x10001)
```

### Create the PEM file

```shell
▶ openssl req -new -sha256 -key dev-root-ca.key -nodes -out dev-root-ca.csr -config rootca.conf
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) []:GB
State or Province Name (full name) []:
Locality Name (eg, city) []:
Organization Name (eg, company) []:
Organizational Unit Name (eg, section) []:
Common Name (eg, fully qualified host name) []:devices.abc.com
Email Address []:

▶ openssl req -x509 -new -in dev-root-ca.csr -key dev-root-ca.key -sha256 -days 365 -out dev-root-ca.pem -config rootca.conf -extensions v3_ca
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) []:GB
State or Province Name (full name) []:
Locality Name (eg, city) []:
Organization Name (eg, company) []:
Organizational Unit Name (eg, section) []:
Common Name (eg, fully qualified host name) []:devices.abc.com
Email Address []:
```

The `days` parameter is set to expire in a year.  The `Common Name` attribute should be some URI, does not need to exist. 

### Generate and Sign the Device Certificate

```shell
▶ openssl genrsa -out device.key 4096      
Generating RSA private key, 4096 bit long modulus
........................................................................++
................................++
e is 65537 (0x10001)
▶ openssl req -new -key device.key -out device.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) []:GB
State or Province Name (full name) []:
Locality Name (eg, city) []:
Organization Name (eg, company) []:
Organizational Unit Name (eg, section) []:
Common Name (eg, fully qualified host name) []:OT-AC8684B5AA8C         
Email Address []:

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
```

You will notice that the `Common Name` now contains the device Id that we retrieved earlier.  You can add a password if you like.  Its more secure if you do.

Now lets generate the x509 certificate that will be placed on the device.

```shell
▶ openssl x509 -req -in device.csr -CA dev-root-ca.pem -CAkey dev-root-ca.key \
-CAcreateserial -out device-cert.pem
Signature ok
subject=/C=GB/CN=OT-AC8684B5AA8C
Getting CA Private Key
```

You should now have the following files.

```shell
▶ ls -lah                                                                                                         
total 48
drwxr-xr-x   8 richardclarke  staff   256B 21 Aug 11:43 .
drwxr-xr-x  13 richardclarke  staff   416B 21 Aug 11:28 ..
-rw-r--r--   1 richardclarke  staff   3.2K 21 Aug 11:29 dev-root-ca.key
-rw-r--r--   1 richardclarke  staff   1.7K 21 Aug 11:33 dev-root-ca.pem
-rw-r--r--   1 richardclarke  staff    17B 21 Aug 11:46 dev-root-ca.srl
-rw-r--r--   1 richardclarke  staff   1.7K 21 Aug 11:46 device-cert.pem
-rw-r--r--   1 richardclarke  staff   1.6K 21 Aug 11:40 device.csr
-rw-r--r--   1 richardclarke  staff   3.2K 21 Aug 11:38 device.key
```

Copy the `device-cert.pem` and `device.key` to `<clone repo>/firmware/data/cloud` folder.

Once this done you will need to `Upload File System image` to the device.  As before this is explained in the [Device Development](./FirmwareDevelopment.md) article.

You are now ready for the cloud part.

## Cloud Part

Each cloud provider registers the CA certificate in a different way.

### Azure IoT Hub

Make sure you have the Azure Cli installed, these are the [installation instructions](https://docs.microsoft.com/en-us/cli/azure/install-azure-cli?view=azure-cli-latest).  

I am assuming you have already created an Azure Subscription.   To set the subscription in the command use the following,

```shell
▶ az account set --subscription "<subscription name>" 
```

There can only be one free IoT Hub in a subscription, so if you already created one, either use the existing IoT Hub or create a new one with S1 tier.  This will be about $25 per month at the time of writing this article.

So with that being said let create an IoT hub in the subscription selected.  The first step is to create a `Resource Group`.  Resource groups are create to isolate all resources to single group.  You can apply subscriptions and RBAC to them to control who has access etc.  Thats the short story, look at the various Azure documentation about them.

Resource groups need a default location where the resource be created.  I live the UK so I am going to use one of those regions.   To list out the location/regions use this command.

```shell
▶ az account list-locations --query "[?contains(regionalDisplayName, 'UK')].name"
[
  "uksouth",
  "ukwest"
]
```

I will be using the `ukwest` location for this article.

```shell
▶ az group create --name dev-ot-rg --location ukwest
{
  "id": "/subscriptions/<subscription id>/resourceGroups/dev-ot-rg",
  "location": "ukwest",
  "managedBy": null,
  "name": "dev-ot-rg",
  "properties": {
    "provisioningState": "Succeeded"
  },
  "tags": null,
  "type": "Microsoft.Resources/resourceGroups"
}
```

Now we have the group we can create the IoT hub.  The hub name is unique globally, so choose your own.

```shell
▶ az iot hub create --name dev-ot-iot-hub --resource-group dev-ot-rg --sku S1
{- Finished ..
  "etag": "AAAAASlLtPs=",
  "id": "/subscriptions/<subscription id>/resourceGroups/dev-ot-rg/providers/Microsoft.Devices/IotHubs/dev-ot-iot-hub",
  "location": "ukwest",
  "name": "dev-ot-iot-hub",
  "properties": {
  	...
    "eventHubEndpoints": {
      "events": {
        "endpoint": "sb://iothub-ns-dev-ot-iot-<uniqueid>.servicebus.windows.net/",
        "partitionCount": 4,
        "partitionIds": [
          "0",
          "1",
          "2",
          "3"
        ],
        "path": "dev-ot-iot-hub",
        "retentionTimeInDays": 1
      }
    },
    "features": "None",
    "hostName": "dev-ot-iot-hub.azure-devices.net",
    "ipFilterRules": [],
    "locations": [
      {
        "location": "UK West",
        "role": "primary"
      },
      {
        "location": "UK South",
        "role": "secondary"
      }
    ],
		....
    "state": "Active",
    ....
  },
  "resourcegroup": "dev-ot-rg",
  "sku": {
    "capacity": 1,
    "name": "S1",
    "tier": "Standard"
  },
  "subscriptionid": "<subscription id>",
  "tags": {},
  "type": "Microsoft.Devices/IotHubs"
}
```

I have removed a lot of the output and just showing the important bits.  The `hostname` may or may not exists as I will be removing it when I finish these articles.

Now we are ready to upload the CA certificate we create earlier.

```shell
▶ az iot hub certificate create --hub-name dev-ot-iot-hub --name dev-root-ca --path ./dev-root-ca.pem
{
  "etag": "AAAAATH/6c8=",
  "id": "/subscriptions/<subscription id>/resourceGroups/dev-ot-rg/providers/Microsoft.Devices/IotHubs/dev-ot-iot-hub/certificates/dev-root-ca",
  "name": "dev-root-ca",
  "properties": {
    "certificate": null,
    "created": "2020-08-22T13:42:24+00:00",
    "expiry": "2021-08-22T13:40:00+00:00",
    "isVerified": false,
    "subject": "devices.abc.com",
    "thumbprint": "A6E664EC2C1547A2B12924582C02378E68DCDF0B",
    "updated": "2020-08-22T13:42:24+00:00"
  },
  "resourceGroup": "dev-ot-rg",
  "type": "Microsoft.Devices/IotHubs/Certificates"
}
```

Now if you look at the `isVerified` property you notice that is currently `false`.  It needs to be `true` before we can use it.  So do that we need to generate a verification code that Azure will recognise.  You will need the `etag` property above to commplete this command.

```shell
▶ az iot hub certificate generate-verification-code --hub-name dev-ot-iot-hub --name dev-root-ca --etag AAAAATH/6c8=
{
  "etag": "AAAAATICxnI=",
  "id": "/subscriptions/<subscription id>/resourceGroups/dev-ot-rg/providers/Microsoft.Devices/IotHubs/dev-ot-iot-hub/certificates/dev-root-ca",
  "name": "dev-root-ca",
  "properties": {
    "certificate": "...",
    "created": "2020-08-22T13:42:24+00:00",
    "expiry": "2021-08-22T13:40:00+00:00",
    "isVerified": false,
    "subject": "devices.abc.com",
    "thumbprint": "A6E664EC2C1547A2B12924582C02378E68DCDF0B",
    "updated": "2020-08-22T13:43:33+00:00",
    "verificationCode": "2978D21A958D22D37BD1747FEF07B538183E3FE6A21B6174"
  },
  "resourceGroup": "dev-ot-rg",
  "type": "Microsoft.Devices/IotHubs/Certificates"
}
```

The `vertificationCode` is used as a `CN` for the certifcate we are going to generate.  Make a note of the `etag` as well.

```shell
▶ openssl req -new -key dev-root-ca.key -out verification-azure.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) []:GB
State or Province Name (full name) []:
Locality Name (eg, city) []:
Organization Name (eg, company) []:
Organizational Unit Name (eg, section) []:
Common Name (eg, fully qualified host name) []:2978D21A958D22D37BD1747FEF07B538183E3FE6A21B6174
Email Address []:

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
```

Now we have `code signing request` lets generate the actual certificate

```shell
▶ openssl x509 -req -in verification.csr -CA dev-root-ca.pem -CAkey dev-root-ca.key -CAcreateserial -out verification.pem
Signature ok
subject=/C=GB/CN=5BA5E596CE80C50C3053D29BB9593C382292BD94C99A7D05
Getting CA Private Key
```

The verification certificate is now ready to be upload to Azure.

```shell
▶ az iot hub certificate verify --etag AAAAATICxnI= --hub-name dev-ot-iot-hub --path ./verification-azure.pem --name dev-root-ca
{
  "etag": "AAAAATIHh+w=",
  "id": "/subscriptions/6df7a8fe-3186-4cef-a025-420fbd6b14b7/resourceGroups/dev-ot-rg/providers/Microsoft.Devices/IotHubs/dev-ot-iot-hub/certificates/dev-root-ca",
  "name": "dev-root-ca",
  "properties": {
    "certificate": null,
    "created": "2020-08-22T13:42:24+00:00",
    "expiry": "2021-08-22T13:40:00+00:00",
    "isVerified": true,
    "subject": "devices.abc.com",
    "thumbprint": "A6E664EC2C1547A2B12924582C02378E68DCDF0B",
    "updated": "2020-08-22T13:46:51+00:00"
  },
  "resourceGroup": "dev-ot-rg",
  "type": "Microsoft.Devices/IotHubs/Certificates"
}
```

So now we can register the devices on Azure without having to create new certificates.  Just create the device with the device id as the name.  This will be covered [IoT Device Registration]() article.

### AWS IoT Core

Now we can do the same with AWS IoT core as we did with Azure.  Make sure you have the AWS Cli installed, these are the [installation instructions](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html)

There is no real concept of a resource group, but you can tag resources which we will do.  There is one IoT Core per region per account.   Now we will use the UK region.  We can use the following command.

```shell
▶ aws ec2 describe-regions --filters 'Name=endpoint,Values=*eu*'
```

Now the issue here is that the output is on a different stream.  Once reviews press `q` to quit the stream.

```
REGIONS ec2.eu-north-1.amazonaws.com    opt-in-not-required     eu-north-1
REGIONS ec2.eu-west-3.amazonaws.com     opt-in-not-required     eu-west-3
REGIONS ec2.eu-west-2.amazonaws.com     opt-in-not-required     eu-west-2
REGIONS ec2.eu-west-1.amazonaws.com     opt-in-not-required     eu-west-1
REGIONS ec2.eu-central-1.amazonaws.com  opt-in-not-required     eu-central-1
```

Now London is actually `eu-west-2` so that the region code we will use.  Unlike Azure we can create a registration/verification code before upload the CA certificate. 

```shell
▶ aws iot get-registration-code

{
    "registrationCode": "d73cf3cc2ef642b380d46e98d8d715febc335c5f24db3b12e13154466a88d1f7"
}
```

Lets generate a key that the verification cert can use.

```shell
▶ openssl genrsa -out verification-aws.key 2048
Generating RSA private key, 2048 bit long modulus
.................+++
......................+++
e is 65537 (0x10001)
```

To create the CA vertification certificate use the following and make sure the `CN` is the `registratonCode` from the above output.

```shell
▶ openssl req -new -key verification-aws.key -out verification-aws.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) []:
State or Province Name (full name) []:
Locality Name (eg, city) []:
Organization Name (eg, company) []:
Organizational Unit Name (eg, section) []:
Common Name (eg, fully qualified host name) []:d73cf3cc2ef642b380d46e98d8d715febc335c5f24db3b12e13154466a88d1f7
Email Address []:

Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:
```

Again we have the `code signing request` lets generate the actual certificate

```shell
▶ openssl x509 -req -in verification-aws.csr -CA dev-root-ca.pem -CAkey dev-root-ca.key -CAcreateserial -out verification-aws.pem -days 365 -sha256
Signature ok
subject=/CN=d73cf3cc2ef642b380d46e98d8d715febc335c5f24db3b12e13154466a88d1f7
Getting CA Private Key
```

Finally we can register the CA and Verification certs in one go.

```shell
▶ aws iot register-ca-certificate --ca-certificate ./dev-root-ca.pem --verification-certificate ./verification-aws.pem --set-as-active --allow-auto-registration
```

Now this command should work, but I have not been able to get it.  If I register the same files via the web console.  Then it loads and registers fine.  I will keep investigating but for the time being we have the CA on both Azure and AWS.

