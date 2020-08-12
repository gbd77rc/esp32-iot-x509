#ifndef CONFIGCLASS_H
#define CONFIGCLASS_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

class BaseConfigInfoClass
{
    public:
        /**
         * Base Class Constructor
         * 
         * @param sectionName The section name which is related to the JSON element in the configuration file
         */    
        BaseConfigInfoClass(const char* sectionName){
            strcpy(this->_sectionName, sectionName);
        }
        /**
         * Virtual save JSON element to the JSON document
         * 
         * @param json The ArduinoJson object that this element will be added to.
         */   
        virtual void save(JsonObject json) = 0;
        /**
         * Virtual load JSON element into the sensor/configuration instance
         * 
         * @param json The ArduinoJson object that this element will be loaded from
         */          
        virtual void load(JsonObjectConst json) = 0;
        /**
         * Virtual create a JSON element that can be used report current telemetry and/or settings to who ever call it
         * 
         * @param json The ArduinoJson object that this element will be added to.
         */         
        virtual void toJson(JsonObject doc) = 0;

        /**
         * Check to see if the sensor/configuration instance supports this section
         * 
         * @param sectionName The section to check compare
         * @return True if section names are the same or not
         */ 
        bool isSection(const char* sectionName)
        {
            return strcmp(this->_sectionName, sectionName) == 0;
        }

        /**
         * The sensor/configuration instance section name
         * 
         * @return pointer to the section name
         */ 
        const char* getSectionName()
        {
            return this->_sectionName;
        }

        /**
         * Check to see if any of the configuration values have changed or not
         * 
         * @return True if the configuration has changed and we wait it saved
         */ 
        bool hasChanged()
        {
            return this->_changed;
        }
        /**
         * Set the hasChanged flag to false
         */ 
        void hasSaved()
        {
            this->_changed = false;
        }

    protected:
        char _sectionName[24];
        bool _changed;
};

class ConfigClass
{
    public: 
        ~ConfigClass();
        void begin(const char* filename, uint8_t defaultSize = 5, uint16_t maxDocSize = 2048);
        void add(BaseConfigInfoClass* config);
        bool load();
        bool save();
        bool shouldSave();

    private:
        BaseConfigInfoClass** _configs;  // Dynamically Allocated array of configs (should not be more then 10 hopefully)
        uint8_t _total; // How many configs have been added.
        const char* _fileName;
        uint16_t _maxDocSize;
};

extern ConfigClass Configuration;

#endif