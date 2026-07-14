#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board.h"
#include "lcd_i2c.h"
#include "buttons.h"
#include "spiffs_log.h"

/* ---- States, straight from the Task 3A diagram ---- */
typedef enum {
    STATE_IDLE,
    STATE_CATEGORY_SELECTION,
    STATE_DETAIL_ENTRY,
    STATE_CONFIRMATION,
    STATE_SAVING,
    STATE_SYNC_ATTEMPT,
    STATE_SYNC_SUCCESS,
    STATE_SYNC_FAILED
} DeviceState;

/* ---- Events, one per button plus the automatic/simulated ones ---- */
typedef enum {
    EVENT_BTN_UP,
    EVENT_BTN_DOWN,
    EVENT_BTN_SELECT,
    EVENT_BTN_BACK,
    EVENT_SAVE_DONE,
    EVENT_SYNC_ACK,
    EVENT_SYNC_FAIL
} DeviceEvent;

void app_main(void)
{
    printf("STATE MACHINE SKELETON — states and events defined\n");
}