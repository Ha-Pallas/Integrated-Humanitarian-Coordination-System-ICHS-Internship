#ifndef SPIFFS_LOG_H
#define SPIFFS_LOG_H

#include "esp_err.h"

esp_err_t spiffs_log_init(void);
void spiffs_log_write(const char *msg);

#endif