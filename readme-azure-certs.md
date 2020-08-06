# Certification Creation Process

Private CA Root Private Key Certificate

    openssl genrsa -out DNH-TMP-Root-CA.key 4096

Root Certificate

    openssl req -x509 -new -key DNH-TMP-Root-CA.key -sha256 -days 365 -out DNH-TMP-Root-CA.pem

> CN should be luxoft.gtp.com

CA-Signed Cert

    openssl genrsa -out key.pem 4096
    openssl req -new -key key.pem -out leaf.csr

> CN should be device name - DNH-TMP-6C8B7612CFA4

    openssl x509 -req -in leaf.csr -CA DNH-TMP-Root-CA.pem -CAkey DNH-TMP-Root-CA.key -CAcreateserial -out cert.pem

* Upload the Root Cert to Azure (DNH-TMP-Root-CA.pem)
* Generate Verification Code

    openssl req -new -key DNH-TMP-Root-CA.key -out verification.csr

> CN should be the verification code from Azure

    openssl x509 -req -in verification.csr -CA DNH-TMP-Root-CA.pem -CAkey DNH-TMP-Root-CA.key -CAcreateserial -out verification_certificate.pem

Check CN in Cert

    openssl x509 -noout -subject -in key.pem   

Check Expiry in Cert

    openssl x509 -enddate -noout -in leaf_certificate.pem

## Device Provisioning Service

* Create IoT Hub Device Provisioning Service
* Select this newly created resource, select Linked IoT hubs under Settings and click on Add.
* In the Add link to IoT hub page
* IoT hub: Select the IoT hub that you want to link with this Device Provisioning Service instance.
* Access Policy: Select iothubowner.
* Go to Manage Enrollments under Settings and click on Add individual enrollment
  * Select Mechanism as "X.509".
  * Upload device certificate created earlier (leaf_certificate.pem) in place of "Primary Certificate". Leave "Secondary Certificate" blank.
  * Enter the appropriate IoT Hub Device ID. Mark IoT Edge device as "False".
  * Click "Save" at the top.
  * Copy device certificate created earlier (leaf_certificate.pem) to main/certs/.
  * Copy private key (leaf_private_key.pem) to main/certs/.

## Azure Connection

    az login
    az iot hub show-connection-string -n <iot-hub-name>
