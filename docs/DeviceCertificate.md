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

We need a private CA to generate the device certificate and create validation chain.  Normally you would have a signing certificate generated from a recognised CA like [digicert](https://www.digicert.com/).  

> I am not associated them, or have ever brought certificates from them. 

As we don't want the expense, we will create our own and upload that  the cloud, which they allow.  To this make sure you have an `openssl` application installed on your os.  I use a Mac OSX here, but it is available for Windows and Linux.  The command line may differ on Windows.  You will need to do some research on the command if it fails.

The `file names` and `CA Common Name` can be changed to suit your needs.  Just make it consistant.

1. Create the CA Key

```shell
▶ openssl genrsa -out dev-root-ca.key 4096
Generating RSA private key, 4096 bit long modulus
...............................................................................................++
...........................................++
e is 65537 (0x10001)
```

2.  Create the PEM file

```shell
▶ openssl req -x509 -new -key dev-root-ca.key -sha256 -days 365 -out dev-root-ca.pem
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

3. Generate and Sign the Device Certificate

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

You will notice that the `Common Name` now contains the device Id that we retrieved earlier.  You can add a password if you.  Its more secured if you do.

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

