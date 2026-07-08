#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <stdint.h>

void lcd_init(void);
void lcd_cmd(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_set_cursor(int row, int col);
void lcd_print(const char *str);

#endif