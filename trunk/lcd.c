/*
 *  lcd.c
 *  
 *  Template for using the LCD with the robot.
 */

#include "interrupts.c"
#include "lcd.h"
#include "ports.h"

#define MAX_CHAR_COUNT 16

void sys_send_command_lcd(unsigned char operation, unsigned char operand) {
	Ports[M6811_PORTA] CLR_BIT(M6811_BIT4); 
	Ports[M6811_HPRIO] CLR_BIT(M6811_BIT5); /* BIT5 = MDA */  
	Ports[M6811_PORTA] CLR_BIT(M6811_BIT4); 

	Ports[M6811_DDRC]   = 255;

	Ports[M6811_PORTB]  = operation;
	Ports[M6811_PORTCL] = operand;  
	
	Ports[M6811_PORTA] SET_BIT(M6811_BIT4); 
	Ports[M6811_PORTA] CLR_BIT(M6811_BIT4); 
       
	do {  /* wait */ 
		Ports[M6811_DDRC]  = 0; 
		Ports[M6811_PORTB] = 1; 

		Ports[M6811_PORTA] SET_BIT(M6811_BIT4);
		Ports[M6811_PORTA] CLR_BIT(M6811_BIT4);
	} while ((Ports[M6811_PORTC] & M6811_BIT7) == 1);

	Ports[M6811_HPRIO] SET_BIT(M6811_BIT5); /* BIT5 = MDA */  
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
	IV.Reset = _Main; /* register the reset handler */
	while(1);         /* hang around */
	return 0;	
}

