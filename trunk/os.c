/*
 * os.c
 * Operating system. Designed for the Rug Warrior
 * robot with the 68HC11 CPU.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 * 
 * Build using supplied make.bat. Number of soft registers must be set to 0. 
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

#include "os.h" 
#include "lcd.h"           
#include "ports.h"
#include "process.h"
#include "fifo.h" 
#include "semaphore.h"
#include "interrupts.h"
#include "user.h"

char *temp = "          "; 


int main(void) {
	IVReset = Reset;
	while(1);		
	return 0;	
}

void Reset() {
	unsigned int i; 
	
	/* Set the timer prescale factor to 16 (1,1)...Must be set very soon after startup!!! */
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT0);
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT1);

	/* Wait a few thousand cycles for the cpu timing to settle. */   
	for (i = 1; i !=0; i++); 
	
	/* Mask any interrupts during booting. */ 
	OS_DI(); 
	
	OS_Init();

	PPPLen    = 3;
	PPP[0]    = 10;
	PPP[1]    = 15; 
	PPP[2]    = 20; 
	//PPP[3]    = IDLE; 
	PPPMax[0] = 10; 
	PPPMax[1] = 10; 
	PPPMax[2] = 10; 
	//PPPMax[3] = 10; 
	
	/* user.h, user.c: define you processes there. */ 
	ProcessInit(); 

	OS_Start();
}
 
/* Initialize the OS */
void OS_Init(void) {	
	int i; 

	/* Initialize the clock */ 
	Clock    = 0;	
	ClockUpdate(); 

	/* Enable TOI */ 
	/* When the hardware pulse accumulator overflows, update the clock. (Causes intermittent crashes.) */ 
	//IVTOI = ClockUpdateHandler;
	//Ports[M6811_TFLG2] CLR_BIT(M6811_BIT7);
	//Ports[M6811_TMSK2] SET_BIT(M6811_BIT7);	

	DevP       = 0;
	SpoP       = 0; 
	PCurrent   = 0; 
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
	int ppp_next;   /* Queue index of the next periodic process. */ 
	process *p; 	
	time_t t;       /* Time to interrupt. */ 
	

	IVSWI = SwitchToProcess; 
	ppp_next = 0; 

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
				/* Update the next time for the device process to run. */
				if (PCurrent->state == NEW) {
					PCurrent->DevNextRunTime = Clock + (time_t)(PCurrent->Name);
				} 
				else {
					PCurrent->DevNextRunTime += (time_t)(PCurrent->Name);
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

		/* No device processes to run *now*, so try for a periodic process. */ 
		if (PPPLen) {
			/* Determine the maximum time to allot the next periodic process, t. */ 
			if (!t || ((Clock + PPPMax[ppp_next]) <  t)) {
				t = Clock + PPPMax[ppp_next]; 
			}

			/* If the current next process isn't idle, try to look it up. */ 
			if (PPP[ppp_next] != IDLE) {
				PCurrent = GetPeriodicProcessByName(PPP[ppp_next]); 	
			}

			circularIncrement(&ppp_next, PPPLen); 

			/* If a periodic process is ready to run, run it. */ 
			if (PCurrent) {
				SetPreemptionTime(t);
				ContextSwitchToProcess();
				ClockUpdate();
				/* If we used up our time slice, continue to the next process. */ 
				if (t < Clock) { continue; }
				/* Otherwise fall through to schedule a sporadic process or idle time. */ 
				else           { PCurrent = 0; }
				//continue; 
			}
		}
		
		/* If we don't yet know when we have to come back, use MAX_EXECUTION_TIME. */ 
		if (!t) { t = MAX_EXECUTION_TIME; }
		
		/* We're here so we must be idle. Schedule a sporadic process. */ 
		if (SpoP) { 
			PCurrent = SpoP; 
		} 
		/* We're here so we must be idle and there must be no sporadic processes to run. */ 
		else {
			PCurrent = &IdleProcess; 
		}
		SetPreemptionTime(t);	
		ContextSwitchToProcess(); 
		continue;
	} 
}
 
PID OS_Create(void (*f)(void), int arg, unsigned int level, unsigned int n) {	
	process *p; 
	int i; 
	BOOL can_create;
	BOOL I; 
	
	I = CheckInterruptMask(); 
	if (!I) { OS_DI(); }
	
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

	/* If we run out of available process blocks, return INVALIDPID. */ 
	if (!can_create) { 
		if (!I) { OS_EI(); }
		return INVALIDPID; 
	}

	p->Name  = n; 
	p->Level = level;
	p->Arg   = arg;
	p->state = NEW; 	
	p->Next  = 0;
	p->Prev  = 0;  
	p->DevNextRunTime   = 0; 
	p->program_location = f;

	AddToSchedulingQueue(p); 
	
	if (!I) { OS_EI(); }
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
	char *str = "         "; 
	/* Move sporatic process to the end of the Queue */ 
	if (PCurrent->Level == SPORADIC) {
		if (SpoP && SpoP->Next) { 
			SpoP = SpoP->Next; 
		}
	} 
	ContextSwitchToKernel(); 
}

int OS_GetParam() {
	return PCurrent->Arg; 
}

void OS_Abort() {
	asm(" stop "); 
}

