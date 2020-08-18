# Utilities

This library contains various file handling routes. It wraps the [SPIFFS](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html) File system routines for the ESP32.  The functions are contained in a namespace called `Utilities`.

## Example

    // ArduinoJson Load
    File json = Utilities::openFile(this->_fileName);
    if (!json)
    {
        LogInfo.log(LOG_ERROR, F("Loading json error!!!!"));
        return false;
    }

    DynamicJsonDocument doc(1024);
    auto err = deserializeJson(doc, json);
    if (err)
    {
        LogInfo.log(LOG_ERROR, "Loading json error (%s)", err.c_str());
        json.close();
        return false;
    }


    // ArduinoJson Save
    DynamicJsonDocument doc(1024);
    auto json = doc.to<JsonObject>();
    File file = Utilities::openFile(this->_fileName, false);
    if (!file)
    {
        return false;
    }
    size_t saved = serializeJson(doc, file);
    file.close();    

    // Load Text File
    size_t size = Utilities::fileSize("/config.ini");
    if (size > 0)
    {
        char *contents = new char[size];
        Utilities::readFile("/config.ini", contents, size);
    }    

