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
	PPPLen    = 3;
	PPP[0]    = 10;
	PPP[1]    = 15; 
	PPP[2]    = 20; 
	PPPMax[0] = 10; 
	PPPMax[1] = 10; 
	PPPMax[2] = 10; 
	
	_sys_init_lcd(); 
	
	OS_InitSem(S_BUZZ,1);
	
	
	/* Test FIFOS and Semaphores */ 
	OS_Create(Test2, 0, PERIODIC, 10); 
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



void Buzz(void) {
	FIFO f = (FIFO)OS_GetParam(); 
	
	while (1) {
		OS_Wait(S_BUZZ); 
		Ports[M6811_PORTA] SET_BIT(BIT3); 
		Ports[M6811_PORTA] CLR_BIT(BIT3); 
		OS_Signal(S_BUZZ); 
		OS_Yield();
	}
}


void Test2(void) {
	FIFO f;

	OS_InitSem(S_PORTE,1);
	f = OS_InitFiFo(); 

	OS_Create(DetectObject, (int)f, DEVICE, 5);
	//OS_Create(ReadLightSensors, (int)f, DEVICE, 10);
	OS_Create(PrintFIFOInt, (int)f, PERIODIC, 20);
}

void ReadLightSensors(void) {
	FIFO f = (FIFO)OS_GetParam(); 
	int l, r;
	
	while (1) {
		l = 0; 
		r = 0; 
		OS_Wait(S_PORTE); 
		/* Activate A/D Converter...makes pins on Port E analog. */ 
		Ports[M6811_OPTION] SET_BIT(BIT7); 
		/* Delay at least 100 microseconds */ 
		OS_Yield(); 
		
		/* Start converting the right photocell */ 
		Ports[M6811_ADCTL] = 0; 
		/* Wait for conversion to complete. */ 
		while (!(Ports[M6811_ADCTL] & BIT7)) {
			OS_Yield(); 
		}
		r = Ports[M6811_ADR1]; 
		
		/* Start converting the left photocell */ 
		Ports[M6811_ADCTL] = 1; 
		/* Wait for conversion to complete. */ 
		while (!(Ports[M6811_ADCTL] & BIT7)) {
			OS_Yield(); 
		}
		l = Ports[M6811_ADR1]; 
		
		OS_Write(f,l+(r*256)); 
		
		OS_Signal(S_PORTE); 
		OS_Yield();
	}
}


void DetectObject(void) {
	FIFO f = (FIFO)OS_GetParam(); 
	int l, r;
	
	while (1) {
		l = 0; 
		r = 0; 
	
		OS_Wait(S_PORTE); 
		/* Deactivate A/D Converter...makes pins on Port E analog. */ 
		Ports[M6811_OPTION] CLR_BIT(BIT7); 
		/* Delay at least 100 microseconds */ 
		OS_Yield(); 
		
		/* Turn on left emitter */ 
		Ports[M6811_PORTD] SET_BIT(BIT4); 
		/* Delay at least 600 microseconds */ 
		OS_Yield(); 
		/* Check for detection. */ 
		l = Ports[M6811_PORTE];
		/* Turn off emitter */ 
		Ports[M6811_PORTD] SET_BIT(BIT4); 
		OS_Yield(); 
		/* Check for detection. */ 
		l = (l & (Ports[M6811_PORTE] & BIT4))?1:0;
		OS_Yield(); 
		
		/* Turn on Right emitter */ 
		Ports[M6811_PORTD] SET_BIT(BIT5); 
		/* Delay at least 600 microseconds */ 
		OS_Yield(); 
		/* Check for detection. */ 
		r = Ports[M6811_PORTE];
		/* Turn off emitter */ 
		Ports[M6811_PORTD] SET_BIT(BIT5); 
		OS_Yield(); 
		/* Check for detection. */ 
		r = (r & Ports[M6811_PORTE] & BIT4)?1:0;
		
		OS_Write(f,l+(r*256)); 
		
		OS_Signal(S_PORTE); 
		OS_Yield();
	}
}


void PrintFIFOInt (void) {
	FIFO f = (FIFO)OS_GetParam(); 
	char *s = "                "; 
	int fi; 
	int l; 
	int r; 

	while (1) {
		if(OS_Read(f,&fi)) {
			l = fi % 256; 
			r = fi / 256; 
			
			s[9] = r % 10 + '0';  
			s[8] = (r /= 10) % 10 + '0';
			s[7] = (r /= 10) % 10 + '0';
			s[6] = (r /= 10) % 10 + '0';
			
			s[4] = l % 10 + '0';  
			s[3] = (l /= 10) % 10 + '0';
			s[2] = (l /= 10) % 10 + '0';
			s[1] = (l /= 10) % 10 + '0';
		} 
		sys_print_lcd(s);
		OS_Yield(); 
	}
}

/******************/ 

void Test1(void) {
	FIFO f; 


	OS_InitSem(5,1); 
	f = OS_InitFiFo(); 

	OS_Create(Write1,    (int)f, PERIODIC, 10);
	OS_Create(WriteA,    (int)f, PERIODIC, 15);
	OS_Create(PrintFIFO, (int)f, PERIODIC, 20);
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

