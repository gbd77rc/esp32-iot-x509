#ifndef BASESENSORS_H
#define BASESENSORS_H

#include "LogInfo.h"

class BaseSensorClass;
typedef bool (BaseSensorClass::*P2TASK)();

typedef struct sensorStateStruct
{
    char sensorName[10];
    TaskHandle_t taskHandle;
    SemaphoreHandle_t semaphoreFlag;
    bool connected;
    BaseSensorClass *sensor;
} SensorState;

class BaseSensorClass
{
public:
    static void task(void *parameters)
    {
        auto pSensor = (struct sensorStateStruct *)parameters;
        // BaseSensorClass *sensor = static_cast<BaseSensorClass*>(parameters);
        LogInfo.log(LOG_VERBOSE, "Name is %s", pSensor->sensorName);
        // LogInfo.log(LOG_VERBOSE, "Handle is %lu", sensor->taskHandle);
        vTaskSuspend(pSensor->taskHandle);
        for (;;)
        {
            if (xSemaphoreTake(pSensor->semaphoreFlag, portMAX_DELAY))
            {
                LogInfo.log(LOG_VERBOSE, "Resuming %s Task To Run ", pSensor->sensorName);
                if (pSensor->sensor->getIsConnected())
                {
                   pSensor->sensor->taskToRun();
                }
                xSemaphoreGive(pSensor->semaphoreFlag);
                vTaskSuspend(pSensor->taskHandle);
            } else {
                LogInfo.log(LOG_VERBOSE, "Could not get flag for %s", pSensor->sensorName);
            }
            vTaskDelay(50);
        }
    }

    void tick()
    {
        LogInfo.log(LOG_VERBOSE, "Tick for %s - %lu",
                    this->sensorState.sensorName,
                    this->sensorState.taskHandle);
        if (this->getIsConnected() && this->sensorState.taskHandle != NULL)
        {
            vTaskResume(this->sensorState.taskHandle);
        }
    }

    virtual const bool getIsConnected() = 0;
    virtual const bool connect() = 0;
    virtual void begin(SemaphoreHandle_t flag) = 0;
    virtual bool taskToRun() = 0;    

protected:
    SensorState sensorState;
};

#endif