#include "spiffs_log.h"
#include "esp_spiffs.h"
#include <stdio.h>

#define LOG_FILE "/spiffs/log.txt"

esp_err_t spiffs_log_init(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };
    return esp_vfs_spiffs_register(&conf);
}

void spiffs_log_write(const char *msg)
{
    FILE *f = fopen(LOG_FILE, "a");
    if (f == NULL) {
        return;
    }
    fprintf(f, "%s\n", msg);
    fclose(f);
}