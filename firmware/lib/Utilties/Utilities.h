#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>
#include <SPIFFS.h>

namespace Utilities
{
    String readFile(const char *filename);
    size_t readFile(const char *filename, char* buffer, size_t size);
    File openFile(const char* filename, bool readOnly = true);
    size_t fileSize(const char* filename);
}

#endif