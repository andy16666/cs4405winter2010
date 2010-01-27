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
	
	*(volatile unsigned char *)(0xfe) = 0;
	*(volatile unsigned char *)(0xff) = 3;
	__asm__ __volatile__ ("jsr 32");
	
	for (k = 1; k != 0; k++);
	
	*(volatile unsigned char *)(0xfe) = 2;
	while (*text != 0 && i < MAX_CHAR_COUNT) {
		*(volatile unsigned char *)(0xff) = *text;
		__asm__ __volatile__ ("jsr 32");
		text++;
		i++;
	}
	
}

static void _sys_send_command_lcd(void) {
	__asm__ __volatile__ ("
		ldx		#4096
		bclr	0,X	#16
		bclr	60,X	#32
		bclr	0,X		#16
		ldaa	#255
		staa	7,X
ldaa	254
		staa	4,X
		ldab	255
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
	");
}

void _sys_init_lcd(void) {
	void* sys_print_loc =  &_sys_send_command_lcd;
	void* internal_mem = (void *)0x0020;
	
	for (; internal_mem < (void *) 0x00ff; internal_mem++, sys_print_loc++)
		*(unsigned char *)(internal_mem) = *(unsigned char *)(sys_print_loc);

	*(volatile unsigned char *)(0xfe) = 0;
	*(volatile unsigned char *)(0xff) = 15;

	__asm__ __volatile__ ("jsr 32");
}


/* define the interrupt vector - only rti and the reset vector are defined currently */
struct interrupt_vectors __attribute__((section(".vectors"))) vectors = 
{
   unused0:                0x0000,
   unused1:                0x0000,
   unused2:                0x0000,
   unused3:                0x0000,
   unused4:                0x0000,
   unused5:                0x0000,
   unused6:                0x0000,
   unused7:                0x0000,
   unused8:                0x0000,
   unused9:                0x0000, 
   unused10:               0x0000, /* unused interrupt handlers */
   sci_handler:            0x0000, /* sci - unused */
   spi_handler:            0x0000, /* spi - unused */
   acc_overflow_handler:   0x0000, /* acc overflow - right shaft encoder - calib */
   acc_input_handler:      0x0000, /* acc input - right shaft encoder - turning */
   timer_overflow_handler: 0x0000, /* timer overflow handler - unused */
   output5_handler:        0x0000, /* out compare 5 - sound */
   output4_handler:        0x0000, /* out compare 4 - unused */
   output3_handler:        0x0000, /* out compare 3 - unused */
   output2_handler:        0x0000, /* out compare 2 - unused */
   output1_handler:        0x0000, /* out compare 1 - unused */
   capture3_handler:       0x0000, /* in capt 3 -left shaft encoder -calib & turning*/
   capture2_handler:       0x0000, /* in capt 2 - unused */
   capture1_handler:       0x0000, /* in capt 1 - unused */
   irq_handler:            0x0000, /* IRQ - unused */
   xirq_handler:           0x0000, /* XIRQ - unused */
   swi_handler:            0x0000, /* swi - unused */
   illegal_handler:        0x0000, /* illegal -unused */
   cop_fail_handler:       0x0000, /* unused */
   cop_clock_handler:      0x0000, /* unused */
   rtii_handler:           0x0000, /* rti - task switching / timer */
   reset_handler:          (void*) &_Main /* reset vector - go to premain */
};


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
	while(1);			/* hang around */
	return 0;	
}

