#ifndef BASESENSORS_H
#define BASESENSORS_H

#include "LogInfo.h"

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
    BaseSensorClass(const char* sensorName){
        strcpy(this->_name, sensorName);
        _instance.instance = this;
        _instance.id = 1;
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
        LogInfo.log(LOG_VERBOSE, "Name is %s", pSensor->instance->getName());
        vTaskSuspend(pSensor->instance->getHandle());
        for (;;)
        {
            if (xSemaphoreTake(pSensor->instance->getSemaphore(), portMAX_DELAY))
            {
                LogInfo.log(LOG_VERBOSE, "Resuming %s Task To Run ", pSensor->instance->getName());
                if (pSensor->instance->getIsConnected())
                {
                   pSensor->instance->taskToRun();
                }
                xSemaphoreGive(pSensor->instance->getSemaphore());
            } else {
                LogInfo.log(LOG_VERBOSE, "Could not get flag for %s", pSensor->instance->getName());
            }            
            vTaskDelay(50);
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
     * To kick off the task
     */  
    void tick()
    {
        LogInfo.log(LOG_VERBOSE, "Tick for %s Sensor",
                    this->getName());
        if (this->getIsConnected() && this->getHandle() != NULL)
        {
            vTaskResume(this->getHandle());
        }
    }

    /**
     * Is the sensor connected flag
     * 
     * @return True if connected
     */  
    const bool getIsConnected(){
        return this->_connected;
    }   

    /**
     * Get the short name of the sensor
     * 
     * @return short name of the sensor
     */  
    const char* getName(){
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

protected:
    char _name[10];
    TaskHandle_t _taskHandle;
    SemaphoreHandle_t _semaphoreFlag;
    bool _connected;
    SensorInstance _instance;
};

#endif