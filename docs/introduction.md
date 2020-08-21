# IoT Visualization Journey

I am going to produce a series of articles the will take an IoT device, register it with a cloud provider ([AWS](https://aws.amazon.com/) and [Azure](https://azure.microsoft.com/en-us/)), then take the data the device sends and how this in a web application.  I will be making public the source code and documentations in a git hub repository which you can find at [https://github.com/gbd77rc/esp32-iot-x509.git](https://github.com/gbd77rc/esp32-iot-x509.git).  This repository was originally just going to be for x509 registration, but I have extended it the series.

## So Lets Get On With The Journey

There are many short tutorial out in the internet, but nothing to really show a journey on how to get a device securely connected to the cloud and make use of the data in visualization application.  These articles will cover the end to end journey, from device developement, cloud registration, web application deployment and seeing the visualization at the end.  I will try to keep the costs of the cloud down to the minimum by using the free tier where I can and be able to undeploy automatically when I can't.

Each of these articles maybe broken into smaller ones, depending on the size of each one.  I will try to limit them to 10 minutes of reading, just to keep it interesting.  This introduction one will be shorter just to wet your appetite.

## Terminology

Throughout this journey I will introduce some TLA.  Most of the meanings below I accquired from Wikipedia.  Expect this section to change overtime as I create/update more articles in this series.

| Term | Meaning                                                      |
| ---- | ------------------------------------------------------------ |
| SOC  | System on a chip - is an integrated circuit (also known as a "**chip**") that integrates all or most components of a computer or other electronic **system** |
| OTA  | Over The Air - This is process where the firmware on the device can be update/replace via remote communication. |
| TLA  | Three Letter Acronym - way of making a phrase with three words shorter by only using the first letters of the words: Three letter acronyms are very common in the world of computers! |



## The Device Story

We are a manufacturer of the devices which are based on ESP32 chip for this series.  The device will contain the following features, so we have a list of telemetry data points.

* Temperature/Humidity Detecting using DHT-22 sensor
* GPS for outdoor positioning
* Connected WiFi signal strength detection

If we have time we will use the WiFi repeaters around the area to workout the device location.  This is based on the signal strength from each detect SSID.  This will more then likely be visited at the end of the series to show how to modify the code, and how to apply OTA updates.

Each sensor and how to wire them up will be covered in the [Device Sensor Setup](./DeviceSensorSetup.md) blog.

Why multiple different sensors?  Good question, and the answer is that I want to show how to use tasks within Arduino that can run at different times on the core or different core on the ESP32 dual core SOC.

The device will have individual x509 certificate so it can Just-In-Time register with a cloud platform.  Each platform have their own process for Just-In-Time registration.

* [AWS](https://docs.aws.amazon.com/iot/latest/developerguide/jit-provisioning.html)
* [Azure](https://docs.microsoft.com/en-us/azure/iot-dps/about-iot-dps#:~:text=Microsoft%20Azure%20provides%20a%20rich,hub%20without%20requiring%20human%20intervention.)

The device will make use of Shadow/Device Twin features for configuration/connection data and standard topic for telemetry data.  

> It could have used standard topics for telemetry and control messages, but need to show off these extra cloud features.  Control topic messages may be covered in cloud related articles, AWS and Azure seem to do it differently or do they?

### Why NOT Google Cloud Platform (GCP)

I will not cover Google Cloud Platform, as it currently stands.  I believe it is currently not fully featured enough yet.  I expect it to be in the next year, if Google don't do their normal, its not making money so lets kill it mind set.  GCP has survived a few years now so it just may do succeed, also last year it seemed they have given up to compete with AWS as a SAAS platform and gone their own way.  Which I am really glad about.  We don't need another AWS look-a-like, but something different.  Multi-cloud is an option some companies need, and now with GCP we have a 3rd global player.

## The Application Story

Once the device is registered and the cloud platform is receiving telemetry we will switch to how to show this data in a web based application.  This application will make use of [OpenId Connect](https://openid.net/connect/) for authentication and the backend service for authorization.  Both Azure and AWS support OpenId Connect and I will show how to use [Key Cloak](https://www.keycloak.org/) for developing the application before being deployed to the cloud.

### Multi-Tenant Story

This application is fine if its just the manufacturer who wants see the devices/data, but what if they are selling them to the end user.  For that, the application will need to support multi-tenant.  This is will limit the device data to just one tenant and if the tenant gives permission, some of the data can be given back to the manufacturer, like the aggregated telemetry feed.  

Each tenant may have one user or multiple different users assigned, so roles will need to be given each user.  Invitations process will need to be created so that the owner of the tenant can invite other users to manage or just view the data.

## The Deployment Story

Each cloud provider has it own way for deploying resources to themselves, AWS using [CloudFormation](https://aws.amazon.com/cloudformation/) and Azure using [ARM - Azure Resource Manager](https://docs.microsoft.com/en-us/azure/azure-resource-manager/management/overview) for this.  I will cover both.  There are other 3rd party deployment systems that make simple to deploy to either without much change in the templates, like [Terraform](https://www.terraform.io/) from HashiCorp.  I will highlight some of the advantages and disadvantages of using it.

Using the cloud providers serverless and container services will be covered, to show when to use each type and what to lookout for.

## The Testing Story

To keep it shortish I have not included any unit tests, end to end tests, deployment tests and the list can go on and on here.  This is really a separate series are blogs in its own right.  Maybe something I will do in the future.  There are many a tutorial out there.

## Wrap Up

The final part of the journey will highlight varous resources to continue to research this ideas and technology here.  This is not the end, as technology keeps changing at a rapid pace.  The concepts like data ingestion and visualization will stay near enough the same, but how to do it will change.



