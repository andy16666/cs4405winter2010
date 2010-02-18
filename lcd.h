#ifndef __LCD_H__
#define __LCD_H__

#define MAX_CHAR_COUNT 16 

#define LCD_OPERATION *(volatile unsigned char *)(0xfe)
#define LCD_OPERAND	  *(volatile unsigned char *)(0xff)
#define LCD_EXECUTE() __asm__ __volatile__ ("jsr 32" : : : "a","b","x","y","memory")


void _sys_init_lcd();
void sys_print_lcd(char* text);

#endif
