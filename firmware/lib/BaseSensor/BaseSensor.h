#ifndef BASESENSORS_H
#define BASESENSORS_H

#include "LogInfo.h"
#include "NTPInfo.h"
#include "WakeUpInfo.h"

class BaseSensorClass;

typedef struct sensorInstanceStruct
{
    uint8_t id;
    BaseSensorClass *instance;
} SensorInstance;

class BaseSensorClass
{
public:
    /**
     * Base Class Constructor
     * 
     * @param sectionName The sensor short name so we can identify the instance in logs
     */
    BaseSensorClass(const char *sensorName, bool singleThread = false)
    {
        strcpy(this->_name, sensorName);
        _instance.instance = this;
        _instance.id = 1;
        this->_singleThreadOnly = singleThread;
    }

    /**
     * Static task function that is ran.  This will cast the parameters point to a struct 
     * and run the instance taskToRun overridden function
     * 
     * @param parameters The parameters to be passed to the task
     */
    static void task(void *parameters)
    {
        auto pSensor = (struct sensorInstanceStruct *)parameters;
        LogInfo.log(LOG_VERBOSE, "Initializing %s Task and is enabled %s", pSensor->instance->getName(),
                    pSensor->instance->getIsEnabled() ? "Yes" : "No");
        vTaskSuspend(pSensor->instance->getHandle());
        for (;;)
        {
            WakeUp.suspendSleep();
            if (pSensor->instance->getIsEnabled())
            {
                if (xSemaphoreTake(pSensor->instance->getSemaphore(), portMAX_DELAY))
                {
                    if (pSensor->instance->getIsConnected())
                    {
                        if (pSensor->instance->taskToRun())
                        {
                            pSensor->instance->setEpoch();
                        }
                    }
                    xSemaphoreGive(pSensor->instance->getSemaphore());
                }
                else
                {
                    LogInfo.log(LOG_VERBOSE, "Could not get flag for %s", pSensor->instance->getName());
                }
            }
            vTaskDelay(100);
            WakeUp.resumeSleep();
            vTaskSuspend(pSensor->instance->getHandle());
        }
    }

    /**
     * Virtual connect function that individual sensor classes should override.
     * 
     * @return True if connected to the sensor or false.
     */
    virtual const bool connect() = 0;

    /**
     * Virtual begin function will initialise the sensor
     * 
     * @param flag The semaphore flag that will control the access to the hardware.
     */
    virtual void begin(SemaphoreHandle_t flag) = 0;

    /**
     * Virtual the instance task to run and read the sensor data
     * 
     * @return True if successfully read the sensor
     */
    virtual bool taskToRun() = 0;

    /**
     * Virtual get a reading string for the sensor data, this will be used for the display
     * 
     * @return string buffer for the data
     */
    virtual const char *toString() = 0;

    /**
     * Virtual update the enabled flag and make the derived class update the configuration has changed flag
     * 
     * @param flag Is enabled true or false now
     */
    virtual void changeEnabled(bool flag) = 0;

    /**
     * To kick off the task
     */
    void tick()
    {
        if ((millis() - this->_last_read) > this->_sampleRate)
        {
            if (this->getIsConnected())
            {
                if (this->getSingleThreadFlag())
                {
                    WakeUp.suspendSleep();
                    this->taskToRun();
                    WakeUp.resumeSleep();
                }
                else if (this->getHandle() != NULL)
                {
                    vTaskResume(this->getHandle());
                }
            }
        }
    }

    /**
     * Is the sensor connected flag
     * 
     * @return True if connected
     */
    const bool getIsConnected()
    {
        return this->_connected;
    }

    /**
     * Is the sensor enabled flag
     * 
     * @return True if enabled
     */
    const bool getIsEnabled()
    {
        return this->_enabled;
    }

    /**
     * Get the short name of the sensor
     * 
     * @return short name of the sensor
     */
    const char *getName()
    {
        return this->_name;
    }

    /**
     * Get the internal task handle
     * 
     * @return The task handle
     */
    const TaskHandle_t getHandle()
    {
        return this->_taskHandle;
    }

    /**
     * Get the semaphore flag
     * 
     * @return The semaphore flag
     */
    const SemaphoreHandle_t getSemaphore()
    {
        return this->_semaphoreFlag;
    }

    /**
     * Update the timestamp to the current epoch time.
     * 
     * @return The semaphore flag
     */
    void setEpoch()
    {
        this->_epoch_time = NTPInfo.getEpoch();
    }

    /**
     * Get if Single Thread Mode Flag, sometime sensorsneed delaymicroseconds and this blocks and the watch dog triggers.
     * 
     * @return The single thread mode flag
     */
    const bool getSingleThreadFlag()
    {
        return this->_singleThreadOnly;
    }

protected:
    char _name[10];
    char _toString[256];
    TaskHandle_t _taskHandle;
    SemaphoreHandle_t _semaphoreFlag;
    bool _connected;
    bool _enabled;
    uint16_t _sampleRate;
    uint64_t _last_read;
    long _epoch_time;
    bool _singleThreadOnly;
    SensorInstance _instance;
};

#endif