# ESP32 using x509 Certificates

This is the code that will be used within a series of blogs that will explain how to connect ESP32/IoT devices to the public cloud, using only x509 certificates.  Generally most tutorials use passwords or connection strings, or tokens.  The [Introduction](./docs/introduction.md) will outline the what is going to happen and the steps required to do this.

This repo will contain the full source code, except any cloud related security tokens, urls etc.

> **Don't ever be temped to include tokens within a git repo, it does not take much to use it and gain access, and if to much rights (IAM, RBAC) been given then you have a big security hole**.

The following links will take the reader to the sections they are interested in.  

* [Device Sensor Setup](./docs/DeviceSensorSetup.md)
* [Firmware Processing](./docs/DeviceFirmwareProcess.md)
* [Firmware Development](./docs/FirmwareDevelopment.md)
* [x509 creation and hub creation](./docs/DeviceCertificate.md)
* [IoT Device Registration](./docs/DeviceCloudRegistration.md)
* Application to show the data from the device
* Development Setup and source control
* Serverless and Containers
* CI/CD Pipelines
* Multi-Tenants

Some of you may just want to know how to get x509 certificates on the device and link it to AWS or Azure or even both.  Well both is just an idea at the moment, still not done the actual work, but the theory is there.
