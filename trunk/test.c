/*
 * test.c
 *
 * Authors: 
 * 	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>	
 */
#include "test.h"
#include "lcd.h"
#include "os.h"
#include "ports.h"
#include "process.h"

void ProcessInit () {
	PPPLen    = 5;
	PPP[0]    = 10;
	PPP[1]    = 20; 
	PPP[2]    = 30; 
	PPP[3]    = 40;
	PPP[4]    = 50;
	PPPMax[0] = 10; 
	PPPMax[1] = 10; 
	PPPMax[2] = 10; 
	PPPMax[3] = 10; 
	PPPMax[4] = 10; 

	OS_Create(TestMain, 0, SPORADIC, 256);
}

void TestMain(void) {
	FIFO buzz, lcd; 
	
	buzz = OS_InitFiFo(); 
	lcd  = OS_InitFiFo(); 
	
	_sys_init_lcd(); 
	
	OS_InitSem(S_BUZZ,1);
	OS_InitSem(S_BUZZ_FIFO,1); 
	OS_InitSem(S_BUZZ_OUTPUT,1);
	OS_InitSem(S_PORTE,1);
	OS_InitSem(S_LCD,1);
	
	/* Prints the operating system name and plays SOS though the speaker. */ 
	OS_Create(PrintLogo, 0, DEVICE, 800); 
	
	
	/* Servers */ 
	OS_Create(FIFOBuzz, buzz, DEVICE, 800);         /* Beep in morse code, charactars from fifo. */ 
	OS_Create(ReadLightSensors, buzz, DEVICE, 200); /* Write values representing the light sensors into the fifo */ 
	OS_Create(ReadMicrophone,   buzz, DEVICE, 500); /* Write values representing the light sensors into the fifo */ 	
	OS_Create(ReadBumpers, lcd, DEVICE, 10);       /* Reads bumper values into a FIFO, and moves the robot accordingly. */ 
	//OS_Create(PrintBumperValue, lcd, PERIODIC, 40); /* Prints the bumper values to the screen. */ 
}


/* 
Device: 
	Scheduling period = 1/frequency of sound. 
	Parameter = number of oscillations to produce. 
*/ 
void Buzz(void) { 
	int i,l; 
	
	l = OS_GetParam(); 
	
	for(i = 0; i < l; i++) {	
		Ports[M6811_PORTA] SET_BIT(BIT3); 
		OS_Yield(); 
		Ports[M6811_PORTA] CLR_BIT(BIT3); 
		OS_Yield(); 
	}
	OS_Signal(S_BUZZ); 
}

inline void dash() { OS_Wait(S_BUZZ); OS_Create(Buzz, 21,  DEVICE, 11); }
inline void dot()  { OS_Wait(S_BUZZ); OS_Create(Buzz,  8,  DEVICE, 11); }
/* Device: > 500ms */ 
void FIFOBuzz(void) {
	FIFO f;
	int fi; 

	f = (FIFO)OS_GetParam(); 
	
	while (1) {
		if(OS_Read(f,&fi)) {
			OS_Wait(S_BUZZ_OUTPUT); 
			switch((char)fi) {
				case 'a': dot(); dash(); break; 
				case 'b': dash(); dot(); dot(); dot(); break; 
				case 'c': dash(); dot(); dash(); dot(); break; 
				case 'd': dash(); dot(); dot(); break;
				case 'e': dot(); break; 
				case 'f': dot(); dot(); dash(); dot(); break; 
				case 'g': dash(); dash(); dot(); break; 
				case 'h': dot(); dot(); dot(); dot(); break; 
				case 'i': dot(); dot(); break; 
				case 'j': dot(); dash(); dash(); dash(); break; 
				case 'k': dash(); dot(); dash(); break; 
				case 'l': dot(); dash(); dot(); dot(); break; 
				case 'm': dash(); dash(); break; 
				case 'n': dash(); dot(); break; 
				case 'o': dash(); dash(); dash(); break; 
				case 'p': dot(); dash(); dash(); dot(); break; 
				case 'q': dash(); dash(); dot(); dash(); break; 
				case 'r': dot(); dash(); dot(); break; 
				case 's': dot(); dot(); dot(); break; 
				case 't': dash(); break; 
				case 'u': dot(); dot(); dash(); break; 
				case 'v': dot(); dot(); dot(); dash(); break; 
				case 'w': dot(); dash(); dash(); break; 
				case 'x': dash(); dot(); dot(); dash(); break; 
				case 'y': dash(); dot(); dash(); dash(); break; 
				case 'z': dash(); dash(); dot(); dot(); break; 
				case '1': dot(); dash(); dash(); dash(); dash(); break; 
				case '2': dot(); dot(); dash(); dash(); dash(); break; 
				case '3': dot(); dot(); dot(); dash(); dash(); break; 
				case '4': dot(); dot(); dot(); dot(); dash(); break; 
				case '5': dot(); dot(); dot(); dot(); dot(); break; 
				case '6': dash(); dot(); dot(); dot(); dot(); break; 
				case '7': dash(); dash(); dot(); dot(); dot(); break; 
				case '8': dash(); dash(); dash(); dot(); dot(); break; 
				case '9': dash(); dash(); dash(); dash(); dot(); break; 
				case '0': dash(); dash(); dash(); dash(); dash(); break; 
				default: OS_Yield(); break; 
			}
			OS_Signal(S_BUZZ_FIFO); 
			OS_Yield();
			/* When all buzz processes have finished, indicate signal that output is complete. */ 
			OS_Wait(S_BUZZ); 
			OS_Signal(S_BUZZ_OUTPUT); 
			OS_Signal(S_BUZZ); 
		}
		OS_Yield();
	}
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
		
		if (l > 100) {
			OS_Wait(S_BUZZ_FIFO);
			OS_Write(f,'l'); 
		}
		
		if (r > 100) {
			OS_Wait(S_BUZZ_FIFO);
			OS_Write(f,'r'); 
		}
		
		OS_Signal(S_PORTE); 
		OS_Yield();
	}
}

void ReadMicrophone(void) {
	FIFO f = (FIFO)OS_GetParam(); 
	int s;
	
	while (1) {
		s = 0; 
		OS_Wait(S_PORTE); 
		/* Activate A/D Converter...makes pins on Port E analog. */ 
		Ports[M6811_OPTION] SET_BIT(BIT7); 
		/* Delay at least 100 microseconds */ 
		OS_Yield(); 
		
		/* Don't listen while buzzing. */ 
		OS_Wait(S_BUZZ_OUTPUT);
		/* Start converting the microphone. */ 
		Ports[M6811_ADCTL] = 2; 
		OS_Signal(S_BUZZ_OUTPUT);
		/* Wait for conversion to complete. */ 
		while (!(Ports[M6811_ADCTL] & BIT7)) {
			OS_Yield(); 
		}
		s = Ports[M6811_ADR1]; 
		
		
		if (s >= 128) {
			s -= 128; 
		}
		else {
			s = 128 - s; 
		}
		
		if (s > 35) {
			OS_Wait(S_BUZZ_FIFO);
			OS_Write(f,'s'); 
		}
		
		OS_Signal(S_PORTE); 
		OS_Yield();
	}
}

void ReadBumpers(void) {
	FIFO f = (FIFO)OS_GetParam(); 
	int b;
	
	while (1) {
		b = 0; 
		OS_Wait(S_PORTE); 
		/* Activate A/D Converter...makes pins on Port E analog. */ 
		Ports[M6811_OPTION] SET_BIT(BIT7); 
		/* Delay at least 100 microseconds */ 
		OS_Yield(); 
		/* Start converting the bumpers. */ 
		Ports[M6811_ADCTL] = 3; 
		/* Wait for conversion to complete. */ 
		while (!(Ports[M6811_ADCTL] & BIT7)) {
			OS_Yield(); 
		}
		b = Ports[M6811_ADR1]; 
		
		OS_Signal(S_PORTE); 
	
		if (b > 3 && b < 7) {
			Ports[M6811_DDRD]  = 0xFF;
			Ports[M6811_PORTD] CLR_BIT(BIT5); // Reverse Right 			

			Ports[M6811_PORTA] CLR_BIT(BIT6); // Left
			Ports[M6811_PORTA] SET_BIT(BIT5); // Right
			
			OS_Write(f,1); 
		}
		else if (b > 23 && b < 26) {
			Ports[M6811_DDRD]  = 0xFF;
			Ports[M6811_PORTD] CLR_BIT(BIT4); // Reverse Left

			Ports[M6811_PORTA] SET_BIT(BIT6); // Left
			Ports[M6811_PORTA] CLR_BIT(BIT5); // Right

			OS_Write(f,2); 
		}
		else if (b > 67 && b < 70) {
			Ports[M6811_DDRD]  = 0xFF;
			Ports[M6811_PORTD] = 0xFF; // Forward
			
			Ports[M6811_PORTA] SET_BIT(BIT6); // Left
			Ports[M6811_PORTA] SET_BIT(BIT5); // Right
						
			OS_Write(f,3); 
		}
		else {
			Ports[M6811_PORTA] CLR_BIT(BIT6); 
			Ports[M6811_PORTA] CLR_BIT(BIT5); 
		
			OS_Write(f,4);  
		}
	
		OS_Yield();
	}
}


void PrintString (void) {
	/* Printing takes a long time. Don't do it while buzzing because it will introduce long pauses. */ 
	OS_Wait(S_BUZZ_OUTPUT); 
	sys_print_lcd((char *)OS_GetParam()); 
	OS_Signal(S_BUZZ_OUTPUT); 	
	OS_Signal(S_LCD); 
}


void PrintBumperValue(void) {
	FIFO f = (FIFO)OS_GetParam(); 
	int state, last; 
	
	last = 0; 
	
	while (1) {
		while(!OS_Read(f,&state)) { OS_Yield(); }
		
		if (!last || (state != last)) {
			if (state == 1) {
				OS_Wait(S_LCD); 
				OS_Create(PrintString, "Front Right", PERIODIC, 50); 
			}
			else if (state == 2) {
				OS_Wait(S_LCD); 
				OS_Create(PrintString, "Front Left", PERIODIC, 50); 
			}
			else if (state == 3) {
				OS_Wait(S_LCD); 
				OS_Create(PrintString, "Rear", PERIODIC, 50); 
			}
			else if (state == 4) {
				OS_Wait(S_LCD); 
				OS_Create(PrintString, "                ", PERIODIC, 50); 
			}
		}
		
		last = state; 	
	}
}



/**************************/

void PrintLogo(void) {
	FIFO f; 
	
	OS_InitSem(S_LOGO,1); 
	f = OS_InitFiFo(); 

	OS_Wait(S_LOGO); 
	OS_Create(WriteA,    (int)f, PERIODIC, 10);
	OS_Create(Write1,    (int)f, PERIODIC, 20);
	OS_Create(PrintFIFO, (int)f, PERIODIC, 30);
	
	OS_Wait(S_BUZZ_OUTPUT);
	dot();
	dot(); 
	dot();
	dash();
	dash();
	dash();
	dot();
	dot();
	dot();
	OS_Signal(S_BUZZ_OUTPUT);
}

void PrintFIFO (void) {
	FIFO f = (FIFO)OS_GetParam(); 
	char *s = "               "; 
	int i; 
	int fi; 
	
	i = 0; 
	while(i < 6) {
		if(OS_Read(f,&fi)) {
			s[i++] = (char)fi; 
		} 
		else {
			OS_Yield(); 
		}
	}
	OS_Wait(S_LCD); 
	OS_Create(PrintString, (int)s, PERIODIC, 50); 
}

void Write1 (void) {
	FIFO f = (FIFO)OS_GetParam(); 
	
	OS_Wait(S_LOGO); 
	OS_Write(f,(int)'l'); 
	OS_Yield();
	OS_Write(f,(int)'O'); 
	OS_Yield();
	OS_Write(f,(int)'S'); 
	OS_Signal(S_LOGO);
}

void WriteA (void) {
	FIFO f = (FIFO)OS_GetParam();

	OS_Write(f,(int)'J'); 
	OS_Yield(); 
	OS_Write(f,(int)'o'); 
	OS_Yield(); 
	OS_Write(f,(int)'e'); 
	OS_Signal(S_LOGO); 
}

