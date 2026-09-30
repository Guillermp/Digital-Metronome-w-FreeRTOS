#include "lcd_wrapper.h"
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C lcd(0x3F, 16, 2);

void lcd_init(void)
{
    lcd.init();
    lcd.backlight();
}



void lcd_print(const char *text)
{
    lcd.print(text);
}

void lcd_print_double(double value, unsigned char decimals)
{
    lcd.print(value, decimals);
}

void lcd_set_cursor(unsigned char column, unsigned char row)
{
    lcd.setCursor(column, row);
}