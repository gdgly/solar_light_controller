#ifndef _OLED_H_
#define _OLED_H_


#include <stdint.h>
#include <string.h>

typedef unsigned char u8;


void OLED_init(void);
void OLED_On(void);
void OLED_Clear(void);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 Char_Size);
void OLED_ShowString(u8 x, u8 y, char *chr, u8 Char_Size);
int OLED_ShowStringFill(u8 x, u8 y, const char *chr, u8 Char_Size);
void OLED_ShowSignal(uint8_t x, uint8_t y, uint8_t level);

#endif
