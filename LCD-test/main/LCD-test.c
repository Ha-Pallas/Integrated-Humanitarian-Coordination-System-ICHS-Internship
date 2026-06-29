#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define SDA 21
#define SCL 22
#define LCD_ADDR 0x27

int counter = 0;

/* I2C init */
void i2c_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA,
        .scl_io_num = SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

/* send raw byte to LCD expander */
void lcd_write(uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);

    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

/* VERY BASIC INIT (enough to wake LCD) */
void lcd_init() {
    vTaskDelay(pdMS_TO_TICKS(50));

    lcd_write(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write(0x03);
    vTaskDelay(pdMS_TO_TICKS(5));
    lcd_write(0x02); // 4-bit mode
}

/* crude “print” (simple version) */
void lcd_print(const char *str) {
    while (*str) {
        lcd_write(*str++);
    }
}

/* MAIN */
void app_main(void)
{
    i2c_init();

    printf("BOOT OK\n");

    lcd_init();

    while (1) {
        printf("Heartbeat: %d\n", counter);

        lcd_write(0x01); // clear
        lcd_print("Heartbeat:");
        lcd_print("\n");

        char buf[16];
        sprintf(buf, "%d", counter);
        lcd_print(buf);

        counter++;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}