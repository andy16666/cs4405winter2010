/*
 * os.c
 * Core operating system kernel code. Designed for the Rug Warrior
 * robot with the HC11 CPU.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 * 
 * m68hc11-gcc -static -g -mshort  -Wl,-m,m68hc11elfb -O3 -msoft-reg-count=0 process.c semaphore.c fifo.c os.c -o os.elf
 *
 * Process management functions and definitions: 
 *   process.h, process.c
 * Fifos: 
 *   fifo.h, fifo.c
 * Semaphores: 
 *   semaphore.h, semaphore.c 
 * 
 * Please define user processes in user.h, user.c. 
 */
#include "ports.c"
#include "os.h"
#include "process.h"
#include "fifo.h" 
#include "semaphore.h"
#include "interrupts.h"

/* Processes */ 
void PrintChar(void);	
void PrintString(void);
void PrintInit (void); 
void PrintTime (void); 
void PrintSystemTime (void);

int main() {
	unsigned int i; 
	char *test = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 

	/* Set the timer prescale factor to 16 (1,1)...Must be set very soon after startup!!! */
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT0);
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT1);

	/* Wait a few thousand cycles for the cpu timing to settle. */   
	for (i = 1; i !=0; i++); 

	/* Mask any interrupts during booting. */ 
	OS_DI(); 
	

	OS_Init();

	PPPLen   = 2;
	PPP[0]    = 10;
	PPP[1]    = 15; 
	PPPMax[0] = 2; 
	PPPMax[1] = 2; 

	
	OS_InitSem(0,1);
	OS_InitSem(1,1);

	PrintInit(); 
	OS_Create(PrintSystemTime,0,DEVICE,500);
	OS_Create(PrintTime,0,DEVICE,10);
	OS_Create(PrintString,(int)test,SPORADIC,10);
	OS_Create(PrintString,(int)test,SPORADIC,10);
	OS_Create(PrintString,(int)test,SPORADIC,10);
	OS_Create(PrintString,(int)test,SPORADIC,10);
	OS_Create(PrintString,(int)test,SPORADIC,10);
	OS_Create(PrintString,(int)test,SPORADIC,10);
	OS_Create(PrintString,(int)test,SPORADIC,10);
	OS_DI(); 

	OS_Start();
	return 0; 
}
 
/* Initialize the OS */
void OS_Init(void) {	
	int i; 

	/* Initialize the clock */ 
	Clock    = 0;	
	ClockUpdate(); 

	/* Enable TOI */ 
	//IV.TOI = ClockUpdateHandler;
	/* When the hardware pulse accumulator overflows, update the clock. (Causes intermittent crashes.) */ 
	//Ports[M6811_TMSK2] SET_BIT(M6811_BIT7);	


	DevP     = 0;
	SpoP     = 0; 
	PCurrent = 0; 

	PKernel.SP = 0; 
	
	/* Initialize processes */ 
	for (i = 0; i < MAXPROCESS; i++) {
		P[i].pid = INVALIDPID; 
		P[i].Prev = 0; 
		P[i].Next = 0;
		/* Initial stack pointer points at the end of the stack. */ 
		P[i].ISP = &(P[i].Stack[WORKSPACE-1]); 
	}	
	/* Initialize fifos. */ 
	for (i = 0; i < MAXFIFO; i++) {
		Fifos[i].fid = INVALIDFIFO; 
	}
	
	/* Set up the idle process. */ 
	IdleProcess.pid  = INVALIDPID; 
	IdleProcess.Name = IDLE; 
	IdleProcess.ISP  = &(IdleProcess.Stack[WORKSPACE-1]);
	IdleProcess.program_location = &Idle; 
	IdleProcess.state = NEW;
}
 
/* Actually start the OS */
void OS_Start(void) {
	process *p; 
	time_t t;         /* Time to interrupt. */ 
	int ppp_next;     /* Queue index of the next periodic process. */ 

	IVSWI = SwitchToProcess; 

	ppp_next = 0; 

	LastTime = Clock; 

	/* Scheduler. */ 
	while (1) {
		/* Syncronize the software clock with the hardware clock. */ 
		ClockUpdate(); 

		PCurrent = 0;
		p = 0; 
		t = 0;  

		if (DevP) {
			p = DevP; 
			/* Search for a Device process ready to run. */ 
			do { 
				if (p->DevNextRunTime <= Clock) {
					PCurrent = p;
					break; 
				}
			} while ((p = p->Next) && (p != DevP)); 
			
			/* If a device process is ready, run it. */ 
			if (PCurrent) {	
				if (PCurrent->DevNextRunTime) {
					PCurrent->DevNextRunTime += (time_t)(PCurrent->Name);
				}
				else {
					PCurrent->DevNextRunTime = Clock + (time_t)(PCurrent->Name);
				}
				ContextSwitchToProcess(); 
				continue; 
			}			
			/* Find the time of the next device process, t. */ 
			else {
				p = DevP; 
				t = p->DevNextRunTime; 
				while ((p = p->Next) && (p != DevP)) { 
					if (p->DevNextRunTime < t) {
						t = p->DevNextRunTime; 
					}
				} 
			}
		}

		/* No device processes to run now, to try for a periodic process. */ 
		if (PPPLen) {
			/* Determine the time of the next interupt. */ 
			if (!t || ((Clock+PPPMax[ppp_next]) <  t)) {
				t = Clock+PPPMax[ppp_next]; 
			}
			/* If the process isn't idle, try to look it up. */ 
			if (PPP[ppp_next] != IDLE) {
				PCurrent = GetPeriodicProcessByName(PPP[ppp_next]); 	
			}

			/* Increment ppp_next circularly. */
			circularIncrement(&ppp_next, PPPLen); 

			/* If a periodic process is ready to run, run it. */ 
			if (PCurrent) {
				SetPreemptionTime(t);
				ContextSwitchToProcess(); 
				continue; 
			}
		}

		/* If we yet know when we have to come back, use MAX_EXECUTION_TIME. */ 
		if (!t) { t = MAX_EXECUTION_TIME; /*ms*/ }

		/* We're here so we must be idle. Schedule a Sporatic Process. */ 
		/* NOTE: Invalid periodic processes are treated as idle. */ 
		if (SpoP) {
			PCurrent = SpoP; 
		} 
		/* We're here so we must be idle and there must be no sporatic processes to run. */ 
		else      { PCurrent = &IdleProcess; }

		SetPreemptionTime(t);	
		ContextSwitchToProcess(); 
		continue; 
	} 
}
 
void OS_Abort() {
	asm(" stop "); 
}

PID OS_Create(void (*f)(void), int arg, unsigned int level, unsigned int n) {	
	process *p; 
	int i; 
	BOOL can_create; 
	
	OS_DI(); 
	/* Find an available process control block */ 
	can_create = FALSE; 
	for (i = 0; i < MAXPROCESS; i++) { 
		p = &P[i];
		if (p->pid == INVALIDPID) {	
			p->pid = i+1; 
			can_create = TRUE; 
			break; 
		}
	} 

	/* If we run out of available process blocks, Abort. */ 
	if (!can_create) { OS_Abort(); }

	p->Name       = n; 
	p->Level      = level;
	p->Arg        = arg;
	p->state      = NEW; 	

	p->Next  = 0;
	p->Prev  = 0;  

	p->DevNextRunTime = 0; 

	/* Set the initial program counter to the address of the function representing the process. */ 
	p->program_location = f;

	AddToSchedulingQueue(p); 
	
	OS_EI(); 
	return p->pid; 
}
 
void OS_Terminate() {
	OS_DI(); 
	PCurrent->pid = INVALIDPID;

	RemoveFromSchedulingQueue(PCurrent); 

	/* Return to the kernel without saving the context. */ 
	ReturnToKernel(); 
} 

void OS_Yield() {
	/* Move sporatic process to the end of the Queue */ 
	if (PCurrent->Level == SPORADIC) { 
		if (SpoP->Next) {
			SpoP = SpoP->Next; 
		}
	} 
	ContextSwitchToKernel(); 
}

int OS_GetParam() {
	return PCurrent->Arg; 
}


/* ########################################*/ 

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

void PrintString (void) {
	char *msg, *msgp;
	FIFO f; 

	msg = (char *)OS_GetParam(); 
	msgp = msg; 

	f = OS_InitFiFo(); 

	OS_Wait(1); 
	OS_Yield(); 	
	OS_Create(PrintChar,(int)f,DEVICE,1);
	while (*msgp) {
		OS_Write(f,(int)(*msgp++)); 
		OS_Yield(); 
	}
	OS_Write(f,0); 
	OS_Signal(1); 
	OS_Terminate(); 
}

void PrintChar (void) {
	FIFO f; 
	char c; 
	int ic; 

	f = (FIFO)OS_GetParam();

	/* Listen for chars in the given FIFO and print them when they appear. */ 
	while (1) {
		while (OS_Read(f,&ic)) {
			c = (char)ic;
			if (!c) OS_Terminate(); 
			OS_Wait(0); 	
			while (!(Ports[M6811_SCSR] & M6811_TDRE))
				continue;
			Ports[M6811_SCDR] = c;
			Ports[M6811_SCCR2] |= M6811_TE;
			OS_Signal(0);
		}
		OS_Yield(); 
	}
}

void PrintTime (void) { 
	time_t time = 0; 
	char *time_p; 
	char *time_s = "00:00.00\r"; 

	int cs; 
	int s; 
	int m; 
	
	
	/* Listen for chars in the given FIFO and print them when they appear. */ 
	while (1) {
		cs = time % 100; 
		s = (time/100) % 60; 
		m = (time/100) / 60; 

		time_s[7] = cs % 10 + '0';  
		time_s[6] = cs / 10 + '0';  
		time_s[4] = s % 10 + '0';  
		time_s[3] = s / 10 + '0';  
		time_s[1] = m % 10 + '0';  
		time_s[0] = m / 10 + '0';  



		OS_Wait(0); 
		time_p = time_s; 
		while (*time_p) {
			while (!(Ports[M6811_SCSR] & M6811_TDRE))
				continue;
			Ports[M6811_SCDR] = *time_p++;
			Ports[M6811_SCCR2] |= M6811_TE;
		}
		OS_Signal(0);

		OS_Yield(); 			
		time++; 
	}
}

/* Specific to this implementation. */ 
void PrintSystemTime (void) { 
	unsigned long time = 0; 
	char *time_p; 
	char *time_s = "        S00:00\r"; 

	int s; 
	int m; 
	
	/* Listen for chars in the given FIFO and print them when they appear. */ 
	while (1) {
		OS_DI(); 	
		time = Clock/1000; 
		OS_EI(); 

		s = time % 60; 
		m = time / 60; 

		time_s[4+9] = s % 10 + '0';  
		time_s[3+9] = s / 10 + '0';  
		time_s[1+9] = m % 10 + '0';  
		time_s[0+9] = m / 10 + '0';  



		OS_Wait(0); 	
		time_p = time_s; 
		while (*time_p) {
			while (!(Ports[M6811_SCSR] & M6811_TDRE))
				continue;
			Ports[M6811_SCDR] = *time_p++;
			Ports[M6811_SCCR2] |= M6811_TE;
		}
		OS_Signal(0);

		OS_Yield(); 
	}
}

