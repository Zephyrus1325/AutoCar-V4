#include "file_read.h"
#include "Arduino.h"
#include <FS.h>
#include <LittleFS.h>
#include "util/logger.h"

void setupFileSystem(){
    
    if(!LittleFS.begin()){
        print_error("LittleFS Mounting Failed");
    }
}

JsonDocument getConfigData(){
    File file = LittleFS.open("/config.json", FILE_READ);
    
    JsonDocument output;

    if(!file){
        print_error("config.json does not exist - Add it asap you idiot");
        return output;
    }

    

    DeserializationError error = deserializeJson(output, file);

    file.close();

    if(error){
        print_error("Config deserialization failed.");
        return output;
    }

    return output;
}

void storeConfig(){

}