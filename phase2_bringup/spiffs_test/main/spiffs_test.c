#include <stdio.h>
#include <string.h>
#include "esp_spiffs.h"

void app_main(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    if (esp_vfs_spiffs_register(&conf) != ESP_OK) {
        printf("SPIFFS mount FAILED\n");
        return;
    }

    printf("SPIFFS mounted OK\n");

    // WRITE FIRST FILE
    FILE *f = fopen("/spiffs/report.txt", "w");
    fprintf(f, "report_001: TEST DATA\n");
    fclose(f);

    printf("WRITE DONE\n");

    // READ FIRST TIME
    char buf[128];

    f = fopen("/spiffs/report.txt", "r");
    fgets(buf, sizeof(buf), f);
    fclose(f);

    printf("READ: %s", buf);

    // APPEND SECOND LINE
    f = fopen("/spiffs/report.txt", "a");
    fprintf(f, "report_002: MORE DATA\n");
    fclose(f);

    printf("APPEND DONE\n");

    // READ FULL FILE
    printf("FULL FILE:\n");

    f = fopen("/spiffs/report.txt", "r");

    while (fgets(buf, sizeof(buf), f)) {
        printf("%s", buf);
    }

    fclose(f);
}