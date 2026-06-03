#ifndef PMDB16_LCD_H
#define PMDB16_LCD_H

// PMDB16 16x2 LCD driver — 4-bit parallel interface
// Version 1.2 — STM32F4 Discovery board pin mapping:
//   RS=PB2, E=PB1, D4-D7=PB12-PB15, Backlight=PA4

#include <stdint.h>

void lcd_initialize(void);
void lcd_clear(void);
void setCursor(uint8_t col, uint8_t row);
void lcd_print(char *string);
void lcd_println(char *string, uint8_t row);
void lcd_drawBar(int value);
void lcd_backlight_ON(void);
void lcd_backlight_OFF(void);

#endif /* PMDB16_LCD_H */
