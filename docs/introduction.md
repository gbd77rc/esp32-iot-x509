# First Blog - What are we going todo

This introduction blog will outline the use case story, and how we are going to do it.  The following links will take the reader to the sections they are interested in.  Some of you may just want to know how to get x509 certificates on the device and link it to AWS or Azure or even both.  Well the the both is just an idea at the moment, still not done the actual work, but the theory is there.

* [Device Sensor Setup](./DeviceSensorSetup.md)
* Firmware Development
* x509 creation
* Azure IoT Hub device registration
* AWS IoT core device registration
* Application to show the data from the device
* Development Setup and source control
* Serverless and Containers
* CI/CD Pipelines
* Multitenants

There are many short tutorial out in the internet, but nothing to really show a journey on how to get a device securely connected to the cloud and make use of the data in visualization application.  It will cover the developement and CI/CD processing in a cloud enabled ecosystem.  I will try to keep the costs of the cloud down to the miniumum by using the free tier where I can and be able to undeploy automatically when I can't.

Each of these blogs maybe broken into smaller ones, depending on the size of each one.  I will try to limit them to 10 minutes of reading, just to keep it interesting.  This introduction one will be shorter just to wet your appetite.

## The Device Story

We are a manufacturer of the devices which are based on ESP32 chip for this series.  The device will contain the following features, so we have a list of telemetry data points.

* Temperature/Humidity Detecting using DHT-22 sensor
* LiDAR distance sensor
* GPS for outdoor positioning
* WiFi signal strenght detection for indoor positioning

Each sensor and how to wire them up will be covered in the [Device Sensor Setup](./DeviceSensorSetup.md).  

> **TODO Insert Device Image**

The device will have individual x509 certificate so it can Just-In-Time register with a cloud platform.  Each platform have their own process for Just-In-Time registration.

* [AWS](https://docs.aws.amazon.com/iot/latest/developerguide/jit-provisioning.html)
* [Azure](https://docs.microsoft.com/en-us/azure/iot-dps/about-iot-dps#:~:text=Microsoft%20Azure%20provides%20a%20rich,hub%20without%20requiring%20human%20intervention.)

The device will make use of Shadow/Device Twin features for configuration/connection data and standard topic for telemetry data.  

> It could have used standard topics for telemetry and control messages, but need to show off these extra cloud features.  Control topic messages will be covered in cloud related blogs, AWS and Azure seem to do it differently or do they?

### Why NOT Google Cloud Platform (GCP)

I will not cover Google Cloud Platform, as it currently stands.  I believe it is currently not fully featured yet.  I expect it to be in the next year, if Google don't do their normal, its not making money so lets close it mind set.  GCP has survived a few years now so it just may do it, also last year it seemed they have given up to compete with AWS as a SAAS platform and gone thier own way.  Which I am really glad about.  We don't need another AWS look-a-like, but something different.

## The Application Story

Once the device is registered and the cloud platform is recieving telemetry we will switch to how to show this data in a web based application.  This application will make use of [OpenId Connect](https://openid.net/connect/) for authentication and the backend service for authorisation.  Both Azure and AWS support OpenId Connect and I will show how to use [Key Cloak](https://www.keycloak.org/) for developing the application before being deployed to the cloud.

### MultiTenant Story

This application is fine if its just the manufacturer who will see and want to the devices/data, but what if they are selling them to the end user.  For that the application will need to support multitenant.  This is will limit the device data to juse one tenant and if the tenant gives permission, some of the data can be given back to the manufacturer, like the aggregated telemetry feed.  

Each tenant may have one user or multiple different users assigned, so roles will need to be given each user.  Invitations process will need to be created so that the owner of the tenant can invite other users to manage or just view the data.

## The Deployment Story

Each cloud provider has it own way for deploying resources to themselves, AWS using [CloudFormation](https://aws.amazon.com/cloudformation/) and Azure using [ARM - Azure Resource Manager](https://docs.microsoft.com/en-us/azure/azure-resource-manager/management/overview) for this.  I will cover both.  There are other 3rd party deployment systems that make simple to deploy to either without much change in the templates, like [Terraform](https://www.terraform.io/) from HashiCorp.  I will highlight some of the advantages and disadvantages of using it.

Using the cloud providers serverless and container services will be covered, to show when to use each type and what to lookout for.  