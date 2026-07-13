#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "lcd_i2c.h"


#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22


i2c_master_bus_handle_t bus;


void app_main(void)
{
    printf("LCD TEST START\n");


    /* I2C init */
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };


    ESP_ERROR_CHECK(
        i2c_new_master_bus(&bus_config, &bus)
    );


    /* Initialize LCD device */
    lcd_i2c_init(bus);


    /* LCD init */
    lcd_init();


    int counter = 0;
    char buf[32];


    while (1)
    {
        lcd_cmd(0x01);

        vTaskDelay(pdMS_TO_TICKS(5));


        lcd_set_cursor(0, 0);
        lcd_print("ICHS OK");


        lcd_set_cursor(1, 0);

        snprintf(buf, sizeof(buf), "Counter: %d", counter++);
        lcd_print(buf);


        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}