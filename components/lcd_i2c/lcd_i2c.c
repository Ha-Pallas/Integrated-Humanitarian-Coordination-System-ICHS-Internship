#include "lcd_i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

i2c_master_dev_handle_t lcd;


/* low-level write */
static void lcd_write(uint8_t data)
{
    i2c_master_transmit(lcd, &data, 1, 1000 / portTICK_PERIOD_MS);
}


/* pulse enable */
static void lcd_pulse(uint8_t data)
{
    lcd_write(data | 0x04);   // EN = 1
    vTaskDelay(pdMS_TO_TICKS(1));

    lcd_write(data & ~0x04);  // EN = 0
    vTaskDelay(pdMS_TO_TICKS(1));
}


/* send 4-bit */
static void lcd_send4(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble & 0xF0) | (rs ? 0x01 : 0x00) | 0x08;
    lcd_pulse(data);
}


/* command */
void lcd_cmd(uint8_t cmd)
{
    lcd_send4(cmd & 0xF0, 0);
    lcd_send4((cmd << 4) & 0xF0, 0);
}


/* data */
void lcd_data(uint8_t data)
{
    lcd_send4(data & 0xF0, 1);
    lcd_send4((data << 4) & 0xF0, 1);
}


/* init */
void lcd_init(void)
{
    vTaskDelay(pdMS_TO_TICKS(50));

    lcd_cmd(0x33);
    lcd_cmd(0x32);
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);

    vTaskDelay(pdMS_TO_TICKS(5));
}


/* set cursor */
void lcd_set_cursor(int row, int col)
{
    int addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_cmd(addr);
}


/* print string */
void lcd_print(const char *str)
{
    while (*str)
    {
        lcd_data(*str++);
    }
}