/*
 *  lcd.c
 *  
 *  Template for using the LCD with the robot.
 */


#include "interrupts.h"
#include "lcd.h"

#define MAX_CHAR_COUNT 16
#define RESETV	(*(volatile int *)(0xBFC0 + 0x3E))

extern void _Main();
  
void sys_print_lcd(char* text) {
	unsigned char i = 0;
        unsigned int k;

	_sys_send_command_lcd(0,3); 

	for (k = 1; k != 0; k++);
	
	while (*text != 0 && i < MAX_CHAR_COUNT) {
		_sys_send_command_lcd(2,*text); 
		text++;
		i++;
	}
	
}

static void _sys_send_command_lcd(unsigned short operation, unsigned short operand) {
	asm volatile("
		ldx		#4096          ; $1000...PORTBASE
		bclr	0,X	#16
		bclr	60,X	#32
		bclr	0,X		#16
		ldaa	#255           ; #$ff
		staa	7,X
		ldaa	%0            ; $fe 
		staa	4,X            
		ldab	%1            ; $ff
		stab	3,X
		bset	0,X		#16
		bclr	0,X		#16
		clr		7,X
wait:	ldaa	#1
		staa	4,X
		bset	0,X		#16
		ldab	3,X
		bclr	0,X		#16
		andb	#128
		beq		wait
		
Done:	bset	60,X	#32
	" : : "m" (operation), "m" (operand) : "X","Y","A","B","memory");
}

void _sys_init_lcd(void) {	
	_sys_send_command_lcd(0,15);
}

void _Main(void)
{
  char *ken = "Ken!\0"; 
  char *joey = "Joey\0";
  unsigned int i;

  _sys_init_lcd();
  while (1)
  {
    sys_print_lcd(ken);
    for(i = 1; i != 0; i++);
    for(i = 1; i != 0; i++);
    for(i = 1; i != 0; i++);
    for(i = 1; i != 0; i++);
    sys_print_lcd(joey);
    for(i = 1; i != 0; i++);
    for(i = 1; i != 0; i++);
    for(i = 1; i != 0; i++);
    for(i = 1; i != 0; i++);
  }
	
  return;
}


/* main - simple reset vector initialization.
 * 
 * this is needed to work with the doanloader. Allows use of the
 * reset button when running in 'special test' mode
 *
 * @return values
 * int:  some ANSI spec requires a return from main 
 */
int main(void) {
	RESETV = (unsigned int)&_Main; 	/* register the reset handler */
	while(1);			            /* hang around */
	return 0;	
}

