#ifndef LCD_WRAPPER_H
#define LCD_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

void lcd_init(void);
void lcd_print(const char *text);
void lcd_print_double(double value, unsigned char decimals);
void lcd_set_cursor(unsigned char column, unsigned char row);

#ifdef __cplusplus
}
#endif

#endif