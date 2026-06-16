#ifndef FILE_READER_H
#define FILE_READER_H

#include <ArduinoJson.h>

void setupFileSystem();
JsonDocument getConfigData();
void storeConfig();

#endif