#ifndef CONFIGCLASS_H
#define CONFIGCLASS_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

class BaseConfigInfoClass
{
    public:
        BaseConfigInfoClass(const char* sectionName){
            strcpy(this->_sectionName, sectionName);
        }
        //virtual ~BaseConfigInfoClass(){}
        virtual void save(JsonObject json) = 0;
        virtual void load(JsonObjectConst json) = 0;
        virtual void toJson(JsonObject doc) = 0;
        bool isSection(const char* sectionName)
        {
            return strcmp(this->_sectionName, sectionName) == 0;
        }

        const char* sectionName()
        {
            return this->_sectionName;
        }

        bool hasChanged()
        {
            return this->_changed;
        }

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