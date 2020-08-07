#include "Config.h"
#include "Utilities.h"
#include "LogInfo.h"

ConfigClass::~ConfigClass()
{
    delete[] this->_configs;
}

void ConfigClass::begin(const char *filename, uint8_t defaultSize, uint16_t maxDocSize)
{
    this->_fileName = filename;
    this->_configs = new BaseConfigInfoClass *[defaultSize];
    this->_maxDocSize = maxDocSize;
}

bool ConfigClass::load()
{
    LogInfo.log(LOG_VERBOSE, "Loading configuration (%s)", this->_fileName);
    File json = Utilities::openFile(this->_fileName);
    if (!json)
    {
        LogInfo.log(LOG_ERROR, F("Loading configuration error!!!!"));
        return false;
    }

    DynamicJsonDocument doc(this->_maxDocSize);
    auto err = deserializeJson(doc, json);
    if (err)
    {
        LogInfo.log(LOG_ERROR, "Loading configuration error (%s)", err.c_str());
        json.close();
        return false;
    }

    auto root = doc.as<JsonObject>();
    if (this->_total > 0)
    {
        for (JsonPair kv : root)
        {
            for (uint8_t i = 0; i < this->_total; i++)
            {
                if (this->_configs[i]->isSection(kv.key().c_str()))
                {
                    LogInfo.log(LOG_VERBOSE, "Loading section (%s)", kv.key().c_str());
                    this->_configs[i]->load(kv.value().as<JsonObject>());
                }
            }
        }
        json.close();
        return true;
    }
    json.close();
    return false;
}

bool ConfigClass::save()
{
    DynamicJsonDocument doc(this->_maxDocSize + 100);
    auto json = doc.to<JsonObject>();
    if (this->_total > 0)
    {
        for (uint8_t i = 0; i < this->_total; i++)
        {
            this->_configs[i]->save(json);
        }
        LogInfo.log(LOG_VERBOSE, F("Saving JSON"), json);
        File file = Utilities::openFile(this->_fileName, false);
        if (!file)
        {
            return false;
        }
        size_t saved = serializeJson(doc, file);
        file.close();
        for (uint8_t i = 0; i < this->_total; i++)
        {
            this->_configs[i]->hasSaved();
        }

        return saved > 0;
    }
    return false;
}

bool ConfigClass::shouldSave()
{
    for (uint8_t i = 0; i < this->_total; i++)
    {
        if (this->_configs[i]->hasChanged())
        {
            return true;
        }
    }
    return false;
}

void ConfigClass::add(BaseConfigInfoClass *config)
{
    this->_configs[this->_total++] = config;
}

ConfigClass Configuration;