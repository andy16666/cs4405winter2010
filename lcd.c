/*
 *  lcd.c
 *  
 *  Template for using the LCD with the robot.
 */

#include "interrupts.h"
#include "lcd.h"

#define MAX_CHAR_COUNT 16

void sys_send_command_lcd(unsigned char operation, unsigned char operand) {
	asm volatile(
		"ldx	#4096        \n"  /* $1000 = Port-base.  */ 
		"bclr	0,X	#16  \n"
		"bclr	60,X	#32  \n"
		"bclr	0,X	#16  \n"
		"ldaa	#255         \n"
		"staa	7,X          \n"
		"ldaa	%[operation] \n"
		"staa	4,X          \n"          
		"ldab	%[operand]   \n"         
		"stab	3,X          \n"
		"bset	0,X #16      \n"
		"bclr	0,X #16      \n"
		"clr	7,X          \n"
	"wait:	ldaa	#1           \n"
		"staa	4,X          \n"
		"bset	0,X #16      \n"
		"ldab	3,X          \n"
		"bclr	0,X #16      \n"
		"andb	#128         \n"
		"beq	wait         \n"  
	"Done:	bset	60,X	#32  \n"
		: 
		: [operation] "m" (&operation), [operand] "m" (&operand) 
		: "x","y","a","b","memory"   
	);
}

void sys_print_lcd(char* text) {
	int i;
        unsigned int k;
	
	sys_send_command_lcd(0,3); /* Clear The LCD?? */ 

	for (k = 1; k != 0; k++); 
	
	for (i = 0; text[i] && i < MAX_CHAR_COUNT; i++) {
		sys_send_command_lcd(2,text[i]);
	}	
}

void sys_init_lcd(void) {	
	sys_send_command_lcd(0,15);
}

void _Main(void)
{
  char *ken  = "Ken!"; 
  char *joey = "Joey";
  unsigned int i;

  sys_init_lcd();

  while (1)
  {
    sys_print_lcd(ken);
    for(i = 1; i != 0; i++); for(i = 1; i != 0; i++); for(i = 1; i != 0; i++); for(i = 1; i != 0; i++);
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
	IV.Reset = _Main; /* register the reset handler */
	while(1);         /* hang around */
	return 0;	
}

