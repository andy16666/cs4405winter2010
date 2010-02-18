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
	FIFO f; 

	_sys_init_lcd(); 
	OS_InitSem(5,1); 
	f = OS_InitFiFo(); 
	
	/* Print the clock. */ 
	//OS_Create(PrintTime,    0, DEVICE, 10);
	
	/* Test FIFOS and Semaphores */ 
	OS_Create(Write1,    (int)f, PERIODIC, 10);
	OS_Create(WriteA,    (int)f, PERIODIC, 15);
	OS_Create(PrintFIFO, (int)f, PERIODIC, 20);

}

/* Specific to this implementation. */
void PrintTime (void) {
        time_t time = 0;
        char *time_s = " 00:00.00";

		time_t cs;
        time_t s;
        time_t m;
		
        while (1) {
				time = Clock/10;

				cs = time % 100; 
				time /= 100; 
				s = time % 60;
                m = time / 60;

				time_s[8] = cs % 10 + '0';  
                time_s[7] = (cs / 10) % 10 + '0';
                time_s[5] = s % 10 + '0';  
                time_s[4] = (s / 10) % 10 + '0';  
				time_s[2] = m % 10 + '0';  
                time_s[1] = (m / 10) % 10 + '0';  

				sys_print_lcd(time_s);
                OS_Yield();
        }
}

void PrintFIFO (void) {
	FIFO f = (FIFO)OS_GetParam(); 
	char *s = "0000000000000000"; 
	int fi; 
	int i; 
	
	char c = 'p'; 

	while (1) {
		for (i = 0; i < 12; i++) {
			if(OS_Read(f,&fi)) {
				s[i] = (char)fi; 
			} 
			else {
				i--;
				OS_Yield(); 
			}
		}
		sys_print_lcd(s);
		OS_Yield();
	}
}

void Write1 (void) {
	FIFO f = (FIFO)OS_GetParam(); 
	
	while (1) {
		OS_Wait(5); 
		OS_Write(f,(int)'1'); 
		OS_Yield();
		OS_Write(f,(int)'2'); 
		OS_Yield();
		OS_Write(f,(int)'3'); 
		OS_Signal(5); 
		OS_Yield();
	}
}

void WriteA (void) {
	FIFO f = (FIFO)OS_GetParam(); 
	
	while (1) {
		OS_Wait(5); 
		OS_Write(f,(int)'A'); 
		OS_Yield();
		OS_Write(f,(int)'B'); 
		OS_Yield();
		OS_Write(f,(int)'C'); 
		OS_Signal(5); 
		OS_Yield();
	}
}

