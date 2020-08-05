#include "Utilities.h"

namespace Utilities
{
    String readFile(const char *filename)
    {
        File file = SPIFFS.open(filename);
        String debugLogData;
        if (!file)
        {
            return debugLogData;
        }

        while (file.available())
        {
            debugLogData += char(file.read());
        }
        //debugLogData += '\0';
        file.close();
        return debugLogData;
    } 

    size_t readFile(const char *fileName, char* buffer, size_t size)
    {
        if (SPIFFS.exists(fileName))
        {
            File file = SPIFFS.open(fileName);
            if (file)
            {
                file.read((uint8_t *)buffer, size);
                file.close();  
                buffer[size] = '\0';
            }
        }
        return 0;
    } 


    size_t fileSize(const char* fileName)
    {
        if ( SPIFFS.exists(fileName))
        {
            File file = SPIFFS.open(fileName);
            size_t t = file.size();
            file.close();
            return t;
        }
        return 0;
    }

    File openFile(const char *filename, bool readOnly)
    {
        return SPIFFS.open(filename, readOnly ? "r": "w");
    }     
} // namespace Utilities