#ifndef LOGGER_H
#define LOGGER_H

#include "Arduino.h"

void print_log(String log_text);
void print_alert(String alert_text);
void print_error(String error_text);


#endif // LOGGER_H