#include "lcd.h"
#include "os.h"

void sys_print_lcd(char* text) {
	unsigned int i = 0;
    unsigned int k;
	BOOL I; 
	
	I = CheckInterruptMask(); 
	OS_DI();
	/* Return home */ 	
	LCD_OPERATION = 0;
	LCD_OPERAND   = 1; 
	LCD_EXECUTE(); 
	for (k = 30000; k !=0; k++);
	
	LCD_OPERATION = 2;
	while (*text != 0 && i < MAX_CHAR_COUNT) {
		LCD_OPERAND = *text;
		LCD_EXECUTE(); 
		text++;
		i++;
	}
	for (k = 30000; k !=0; k++);
	if (!I) { OS_EI(); }
}

void sys_clear_lcd() {
	unsigned int i = 0;
    unsigned int k;
	BOOL I; 
	
	I = CheckInterruptMask(); 
	if (!I) { OS_DI(); }
	
	/* Clear */ 	
	LCD_OPERATION = 0;
	LCD_OPERAND   = 1;
	LCD_EXECUTE(); 
	
	for (k = 30000; k !=0; k++);
	
	if (!I) { OS_EI(); }
}

static void _sys_send_command_lcd(void) {
	__asm__ __volatile__ (
		"ldx	#4096\n\t"
		"bclr	0,X	#16\n\t"
		"bclr	60,X	#32\n\t"
		"bclr	0,X	#16\n\t"
		"ldaa	#255\n\t"
		"staa	7,X\n\t"
		"ldaa	254\n\t"
		"staa	4,X\n\t"
		"ldab	255\n\t"
		"stab	3,X\n\t"
		"bset	0,X	#16\n\t"
		"bclr	0,X	#16\n\t"
		"clr	7,X\n\t"
"wait:	ldaa	#1\n\t"
		"staa	4,X\n\t"
		"bset	0,X	#16\n\t"
		"ldab	3,X\n\t"
		"bclr	0,X	#16\n\t"
		"andb	#128\n\t"
		"beq	wait\n\t"
		
"Done:	bset	60,X	#32"
	 : : : "a","b","x","y","memory");
}

void _sys_init_lcd(void) {
	BOOL I; 
	
	I = CheckInterruptMask(); 
	if (!I) { OS_DI(); }
	
	void* sys_print_loc =  &_sys_send_command_lcd;
	void* internal_mem = (void *)0x0020;
	
	for (; internal_mem < (void *) 0x00ff; internal_mem++, sys_print_loc++)
		*(unsigned char *)(internal_mem) = *(unsigned char *)(sys_print_loc);

	LCD_OPERATION = 0;
	LCD_OPERAND = 15;
	LCD_EXECUTE(); 
	
	if (!I) { OS_EI(); }
}
