#include "logger.h"
#include "comms/network_server.h"


inline void print_thing(String str, String type){
    String output = "[" + type + "][" + String(pcTaskGetName(NULL)) + "] " + str;
    
    JsonDocument doc;
    doc["type"] = "telemetry";
    doc["log"] = output;
    doc["msgType"] = type;

    String jsonOut;
    serializeJson(doc, jsonOut);

    Serial.println(output);
    ws.textAll(jsonOut);
    // TODO: Add a way to send log messages via network
}

void print_log(String log_text){
    print_thing(log_text, "LOG");
}

void print_alert(String alert_text){
    print_thing(alert_text, "ALERT");
}

void print_error(String error_text){
    print_thing(error_text, "ERROR");
}
