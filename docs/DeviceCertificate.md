# Device Certificate

Each device should have its own certificate with its `Common Name` set to the device identifier.  So the first step is to work out the device identifier.  If we the manufacturer, then this would be part of the automatic certificate creation process when the device is produced.  As we are not then the quick why is to flash the following code to it.

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

