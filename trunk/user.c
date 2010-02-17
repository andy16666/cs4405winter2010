/*
 * user.c
 *
 * Authors: 
 * 	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>	
 */
#include "user.h"
#include "lcd.h"
#include "os.h"
#include "ports.h"
#include "process.h"

void ProcessInit () {
	char *test = "ABCDEFGH"; 
	char *device = "Device  "; 
	char *periodic = "Periodic"; 
	char *sporadic = "Sporadic"; 

	OS_Create(PrintString2,(int)periodic,PERIODIC,15);
	OS_Create(PrintString2,(int)sporadic,SPORADIC,9);
	OS_Create(PrintString,(int)device,DEVICE,9000);
	OS_Create(PrintString,(int)test,PERIODIC,20);
	OS_Create(PrintSystemTime2,0,DEVICE,1000);
	OS_Create(PrintSystemTime2,0,PERIODIC,10);
	OS_Create(PrintString,(int)test,SPORADIC,19);
}

void PrintInit(void) {
	static BOOL initialized = FALSE; 
	if (!initialized) {
		/* Configure the SCI to send at M6811_DEF_BAUD baud.  */
		Ports[M6811_BAUD] = M6811_DEF_BAUD;

		/* Setup character format 1 start, 8-bits, 1 stop.  */
		Ports[M6811_SCCR1] = 0;

		/* Enable receiver and transmitter.  */
		Ports[M6811_SCCR2] = M6811_TE | M6811_RE;
		
		initialized = TRUE;
	}
}


/* Specific to this implementation. */
void PrintSystemTime2 (void) {
        time_t time = 0;
        char *time_s = "        S00:00";

        time_t s;
        time_t m;
		
        while (1) {
				time = Clock/1000;

				s = time % 60;
                m = time / 60;

                time_s[5+8] = s % 10 + '0';  
                time_s[4+8] = (s / 10) % 10 + '0';  
				time_s[2+8] = m % 10 + '0';  
                time_s[1+8] = (m / 10) % 10 + '0';  

				sys_print_lcd(time_s);
                OS_Yield();
        }
}

void PrintString (void) {
	while (1) {
		sys_print_lcd((char *)OS_GetParam()); 
		OS_Yield(); 
	}
}

void PrintString2 (void) {
	while (1) {
		sys_print_lcd((char *)OS_GetParam()); 
	}
}
