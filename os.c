/*
 * os.c
 * Core operating system kernel code. Designed for the Rug Warrior
 * robot with the HC11 CPU.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 */
 

#include "fifo.h"
#include "interrupts.c"
#include "ports.h"
#include "os.h"

#define M6811_CPU_HZ 40000000 

typedef volatile unsigned long time_t; 

/* 
  Process Table 
*/
typedef struct proc_struct {
	PID pid;  	               /* Process ID. */ 
	unsigned int Name;             /* Name of process */ 
	unsigned int Level;            /* Scheduling level/queue */ 
	int   Arg;                     /* Process argument */ 
	char Stack[WORKSPACE]; 
	char *SP;                     /* Last Stack Pointer */ 
	char *ISP;                    /* Initial Stack Pointer */ 
	void(*program_location)(void); /* Pointer to the process, to start it for the first time. */ 
	BOOL running;                  /* Indicates if the process has been started yet. */ 

	struct proc_struct* QueuePrev;     /* Pointer to the previous process of the same type, for SPORATIC and DEVICES. */ 
	struct proc_struct* QueueNext;     /* Pointer to the next process of the same type. */ 
	
	time_t DevNextRunTime; 	       /* Device processes: run next at this time. */ 
} process;

typedef struct kernel_struct {
	char *SP;                     /* Last Stack Pointer */ 
} kernel; 

/* Safe Interrupt Service Handlers */
void ClockUpdateHandler (void) __attribute__((interrupt)); 
void OC4Handler (void) __attribute__((interrupt)); 


void ContextSwitchToProcess(void); /* Perform a context switch to PCurrent */ 
void ContextSwitchToKernel(void);  /* Perform a context switch to PKernel  */

/* Internal process management helpers. */ 
void ClockUpdate(void); 
void SwitchToProcess(void); 
void ReturnToKernel(void); 
process *GetPeriodicProcessByName(unsigned int n); 
process *QueueAdd(process *p, process *Queue);
process *QueueRemove(process *p, process *Queue);
void SetPreemptionTime(time_t time);   
void SetPreemptionTimerInterval(unsigned int miliseconds); 

/* Processes */ 
void Idle (void); 
void TestProcess (void);
void PrintChar(void);
void HelloWorld (void); 
void Period (void); 
void Third (void); 
void PrintInit (void); 
void Sporadic (void); 

/* Access all ports through this array */ 
//volatile unsigned short Ports[];
//volatile struct interrupt_vectors IV; 

process P[MAXPROCESS]; /* Main process table.    */ 
process *PCurrent;     /* Last Process to run    */ 
process *DevP;         /* Device Process Queue   */ 
process *SpoP;         /* Sproatic Process Queue */

process IdleProcess; 
kernel  PKernel;

int PPPLen;
int PPP[MAXPROCESS]; 
int PPPMax[MAXPROCESS];

fifo_t Fifos[MAXFIFO];    /* FIFOs */

time_t Clock;             /* Approximate time since system start in ms. */ 
unsigned int TimeQuantum; /* Ticks per ms */ 


int main(int argc, char **argv) {		
	OS_DI(); 
		
	OS_Init();

	

	PPPLen   = 3;
	PPP[0]    = IDLE;
	PPP[1]    = 10;
	PPP[2]    = 15;  
	PPPMax[0] = 10; 
	PPPMax[1] = 10; 
	PPPMax[2] = 10;

	PrintInit(); 
	OS_Create(Third,0,DEVICE,30000);
	OS_Create(HelloWorld,0,DEVICE,10000);  
	OS_Create(Period,0,PERIODIC,15);  
	OS_Create(Sporadic,0,SPORADIC,10);  
	OS_DI(); 

	OS_Start(); 

	return 0;
}
 
/* Initialize the OS */
void OS_Init(void) {	
	int i; 

	Clock    = 0;
	DevP     = 0;
	SpoP     = 0; 
	PCurrent = 0; 

	PKernel.SP = 0; 
	
	/* TODO: Set up kernel process. */ 
	/* TODO: Set Kernel Interrupt Vector: PKernel.IV = */
	
	/* Initialize processes */ 
	for (i = 0; i < MAXPROCESS; i++) {
		P[i].pid = INVALIDPID; 
		P[i].QueuePrev = 0; 
		P[i].QueueNext = 0;
		/* Initial stack pointer points at the end of the stack. */ 
		P[i].ISP = &(P[i].Stack[WORKSPACE-1]); 
		/* TODO: Set IdleProcess.IV  =  */ 
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
	IdleProcess.running = FALSE;

	/* When the hardware pulse accumulator overflows, update the clock, clearing the overflow. */ 
	IV.TOI = ClockUpdateHandler;

	/* Set the timer prescale factor to 16 (1,1)...Must be set very soon after startup!!! */
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT0);
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT1);
	/* TODO: Make sure this calculation is correct. */ 
	TimeQuantum = (M6811_CPU_HZ/16)/1000;

	/* Enable TOI */ 
	/*Ports[M6811_TMSK2] SET_BIT(M6811_BIT7);	*/
}
 
/* Actually start the OS */
void OS_Start(void) {
	process *p; 
	time_t t;         /* Time to interrupt. */ 
	int ppp_next;     /* Queue index of the next periodic process. */ 

	ppp_next = 0; 

	IV.SWI = SwitchToProcess; 

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
			} while ((p = p->QueueNext) && (p != DevP)); 
			
			/* If a device process is ready, run it. */ 
			if (PCurrent) {	
				ContextSwitchToProcess(); 
				PCurrent->DevNextRunTime += (time_t)(PCurrent->Name);
				continue; 
			}			
			/* Find the time of the next device process, t. */ 
			else {
				p = DevP; 
				t = p->DevNextRunTime; 
				while ((p = p->QueueNext) && (p != DevP)) { 
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
			ppp_next = (++ppp_next >= PPPLen)?0:ppp_next;

			/* If a periodic process is ready to run, run it. */ 
			if (PCurrent) {
				SetPreemptionTime(t);
				ContextSwitchToProcess(); 
				continue; 
			}
		}

		/* We're here so we must be idle. Schedule a Sporatic Process. */ 
		/* NOTE: Invalid periodic processes are treated as idle. */ 
		if (SpoP) {
			PCurrent = SpoP; 
			if (t) {
				SetPreemptionTime(t);
			}	
			ContextSwitchToProcess(); 
			continue; 
		}
		
		/* We're here so we must be idle and there must be no sporatic processes to run. */ 
		if (t) {
			SetPreemptionTime(t);
			PCurrent = &IdleProcess;
			ContextSwitchToProcess(); 			
			continue; 
		}
	} 
}
 
void OS_Abort() {
	asm(" stop "); 
}

/* Set OC4 to interrupt at a specific absolute time in the future. */ 
void SetPreemptionTime(time_t time) {
	SetPreemptionTimerInterval((unsigned int)(time - Clock)); 
}

/* Set OC4 to interrupt in a number of miliseconds. */ 
void SetPreemptionTimerInterval(unsigned int miliseconds) {
	unsigned int *TOC4_address; 
	unsigned int *timer_address; 
	
	/* Make sure these are read as 16 bit numbers. */ 
	TOC4_address  = (unsigned int*)&(Ports[M6811_TOC4_HIGH]);
	timer_address = (unsigned int*)&(Ports[M6811_TCNT_HIGH]);
	
	*TOC4_address = (*timer_address + (miliseconds * TimeQuantum)); 


	Ports[M6811_TCTL1] SET_BIT(M6811_BIT2);
	
	Ports[M6811_TFLG1] SET_BIT(M6811_BIT4);
	/* Unmask OC4 interrupt */
	Ports[M6811_TMSK1] SET_BIT(M6811_BIT4);
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

	if (!can_create) { OS_Abort(); }

	p->Name       = n; 
	p->Level      = level;
	p->Arg        = arg;
	p->running    = FALSE; 	

	p->DevNextRunTime = 0; 

	/* Set the initial program counter to the address of the function representing the process. */ 
	p->program_location = f;

	/* Add Sporatic Processes to the Sporatic Queue */ 
	if      (p->Level == SPORADIC) { SpoP = QueueAdd(p, SpoP); }
	/* Add Device Processes to the Device Queue */ 
	else if (p->Level == DEVICE)   { DevP = QueueAdd(p, DevP); }
	
	OS_EI(); 
	return p->pid; 
}
 
void OS_Terminate() {
	OS_DI(); 
	PCurrent->pid = INVALIDPID;

	/* Remove the process from the SPORATIC queue */
	if      (PCurrent->Level == SPORADIC) { SpoP = QueueRemove(PCurrent, SpoP); } 
	/* Remove the process from the DEVICE queue */
	else if (PCurrent->Level == DEVICE)   { DevP = QueueRemove(PCurrent, DevP); }
	/* TODO: Release Semiphores  */
	
	/* Return to the kernel without saving the context. */ 
	ReturnToKernel(); 
} 

void OS_Yield() {
	/* Move sporatic process to the end of the Queue */ 
	if (PCurrent->Level == SPORADIC) { 
		if (SpoP->QueueNext) {
			SpoP = SpoP->QueueNext; 
		}
	} 
	ContextSwitchToKernel(); 
}



int OS_GetParam() {
	return PCurrent->Arg; 
}

/* Performs a context switch, saving the given current stack pointer. 
	Used ONLY by:
		- ContextSwitchToKernel 
		- ContextSwitchToProcess
   This function creates a black hole within which we can perform a 
   context switch without it affecting the calling process. 
*/ 

/* Called directly to transfer control to the kernel, saving current context. */ 
void ContextSwitchToKernel(void)  { 
	OS_DI(); 
	/* Interrupt to a SwitchToProcess() or ReturnToKernel() ISRs. */ 
	asm volatile (" swi "); 
	/* This function returns when rti is called. */
	OS_EI();
}
/* Called directly to transfer control to PCurrent, saving current context. */ 
void ContextSwitchToProcess(void) { 
	/* Interrupt to a SwitchToProcess() or ReturnToKernel() ISRs. */ 
	asm volatile (" swi "); 
	/* This function returns when rti is called. */
}


/* Interrupt service routine for updating the clock. */ 
void ClockUpdateHandler (void) { 
	OS_DI(); 
	ClockUpdate(); 
	OS_EI(); 
}

/* 
   Syncronize the software clock with the hardware tick counter. 
   ASSUMPTIONS: 
	- That no more than one overflow occurs between updates. 
	- That interrupts are disabled when this function is called. 
*/ 
void ClockUpdate(void) {
	unsigned int elapsed_time; 
	unsigned int timer_value; 
	volatile unsigned int *timer_address; 
	static unsigned int residual = 0; 
	static unsigned int last_timer_value = 0;
	
	/* Read the timer from the tick register as a single 16 bit number. */ 
	timer_address = (unsigned int *)&Ports[M6811_TCNT_HIGH];
	timer_value = *timer_address;

	/* Check for TOF flag indicating an overflow condition. */ 
	if (Ports[M6811_TFLG2] & M6811_BIT7) {
		/* Clear the overflow condition. */ 
		Ports[M6811_TFLG2] CLR_BIT(M6811_BIT7);
	}

	if (last_timer_value > timer_value) {
		elapsed_time = (0xFFFF - last_timer_value) + timer_value; 
	} else {
		elapsed_time = timer_value - last_timer_value;
	}

	/* Add on the residual from the last update. */ 
	elapsed_time += residual; 

	residual = elapsed_time % TimeQuantum; 

	Clock += (elapsed_time-residual)/TimeQuantum; 

	/* Account for the residual during the next update. */ 
	last_timer_value = timer_value; 
}


void Idle (void) { while (1); }


process *GetPeriodicProcessByName(unsigned int n) {
	int i; 
	process *p; 
	for (i = 0; i < MAXPROCESS; i++) {
		p = &P[i]; 
		if ((p->pid != INVALIDPID) 
		&& (p->Level == PERIODIC) 
		&& (p->Name == n)
		) {
			return p; 
		}		
	}
	return 0; 
}

process *QueueAdd(process *p, process *Queue) {
	/* The graph has one or more nodes. */ 
	if (Queue) {
		/* The graph has more than one existing node. */ 
		if (Queue->QueueNext && Queue->QueuePrev) {
			p->QueuePrev = Queue->QueuePrev; 
			p->QueueNext = Queue; 
			Queue->QueuePrev->QueueNext = p; 
			Queue->QueuePrev = p; 
		} 
		/* The graph has exactly one existing node. */ 
		else {
			Queue->QueueNext = p; 
			Queue->QueuePrev = p;
			p->QueueNext = Queue; 
			p->QueuePrev = Queue;
		}
	} 
	/* The graph has no existing nodes. */ 
	else { 
		Queue = p; 
	} 
	return Queue; 
}

process *QueueRemove(process *p, process *Queue) {
	/* The graph has one or more nodes. */ 
	if (Queue) { 
		/* Queue points to the node to be removed. */ 
		if (Queue == p) { 
			/* There is more than one node */ 
			if (Queue->QueueNext) { Queue = Queue->QueueNext; }
			/* There is only one node and it is the one to be removed. */ 
			else                  { return 0; }
		}
		/* The graph has only two nodes  */ 
		if (p->QueuePrev == p->QueueNext) {
			Queue->QueuePrev = 0; 
			Queue->QueueNext = 0;
		}
		/* The graph has more than two nodes. */ 
		else {
			p->QueuePrev->QueueNext = p->QueueNext; 
			p->QueueNext->QueuePrev = p->QueuePrev; 
		}
	}
	return Queue; 
}

/* Preemption must be handled differently from traps to preserve local variables. */ 
void OC4Handler(void) {
	asm(" swi ");
}

/* 
   Directly return control to kernel. 
   Called ONLY by:
    - SWI from ContextSwitch() 
    - OS_Terminate(). 
*/ 
void ReturnToKernel(void)  {
	OS_DI(); 	
	/* Store the stack pointer in the given location. */ 
	asm volatile (" sts %0 " : "=m" (PCurrent->SP) : : "memory"); 
	/* Correct for function call. */
	PCurrent->SP++; 
	PCurrent->SP++;
	/* Mask OC4 interrupts */
	Ports[M6811_TMSK1] CLR_BIT(M6811_BIT4);
	/* Reset interrupt handlers. */ 
	IV.OC4 = 0; /* The kernel cannot be preempted. */ 
	IV.SWI = SwitchToProcess; 

	/* Load Kernel Stack Pointer */
	asm volatile (" lds %0 " : : "m" (PKernel.SP) : "memory"); 
	/* Return control to the kernel */ 
	asm volatile (" rti "); 
}

/* Transfer control PCurrent. ONLY called by SWI from ContextSwitch() in kernel mode. */ 
void SwitchToProcess(void) {
	 /* Store the stack pointer in the given location. */ 
	asm volatile (" sts %0 " : "=m" (PKernel.SP) : : "memory"); 
	/* Correct for function call. */
	PKernel.SP++; 
	PKernel.SP++;
	/* Set interrupt handlers. */ 
	IV.OC4 = OC4Handler; /* OC4 must jump out to a proper interrupt handler to preserve local variables. */ 
	IV.SWI = ReturnToKernel;
	
	/* If the process has already been running, we can return to its last context. */ 
	if (PCurrent->running) {		
		/* Load Process Stack Pointer */ 
		asm volatile (" lds %0 " : : "m" (PCurrent->SP) : "memory"); 
		/* Return control to running process. */ 
		asm volatile (" rti "); 
	} 
	/* If the process has not been started, we need to start it for the first time. */ 
	else {
		/* Set the process to the running state. */
		PCurrent->running = TRUE; 
		/* Load Process Stack Pointer */ 
		asm volatile (" lds %0 " : : "m" (PCurrent->ISP) : "memory"); 
		/* Run process for the first time. */
		OS_EI();
		PCurrent->program_location();
		OS_DI();
		/* When the process ends run terminate to clean it up. */ 
		OS_Terminate(); 
	}
}


void TestProcess (void) {
	unsigned char i;  

	while (1) {	
		for(i = 1; i != 0; i++); 

		OS_Create(HelloWorld,0,PERIODIC,15);  
		
		OS_Yield(); 
	}
}


void PrintInit(void) {
  /* Configure the SCI to send at M6811_DEF_BAUD baud.  */
  Ports[M6811_BAUD] = M6811_DEF_BAUD;

  /* Setup character format 1 start, 8-bits, 1 stop.  */
  Ports[M6811_SCCR1] = 0;

  /* Enable receiver and transmitter.  */
  Ports[M6811_SCCR2] = M6811_TE | M6811_RE;
}


void HelloWorld (void) {
  char *msg = "Hello world!\n";
  char *msgp; 



	while (1) {
          msgp = msg; 
	  while (*msgp != 0) {
		/* Wait until the SIO has finished to send the character.  */
		while (!(Ports[M6811_SCSR] & M6811_TDRE))
		  continue;
	
		Ports[M6811_SCDR] = *msgp++;
		Ports[M6811_SCCR2] |= M6811_TE;
	  }
	  OS_Yield(); 
	}
}

void Third (void) {
  char *msg = "1/3 as often!\n";
  char *msgp; 

	while (1) {
          msgp = msg; 
	  while (*msgp != 0) {
		/* Wait until the SIO has finished to send the character.  */
		while (!(Ports[M6811_SCSR] & M6811_TDRE))
		  continue;
	
		Ports[M6811_SCDR] = *msgp++;
		Ports[M6811_SCCR2] |= M6811_TE;
	  }
	  OS_Yield(); 
	}
}

void Period (void) {
  char *msg = "Periodic Process!!!"; 
  int i = 0; 
  unsigned int j = 0;
  int len = 18;

  while (1) {
	for (i = 0; i < len; i++) {
		for(j = 1; j != 0; j++);
		OS_DI();
		/* Wait until the SIO has finished to send the character.  */
		while (!(Ports[M6811_SCSR] & M6811_TDRE))
		  continue;
	
		Ports[M6811_SCDR] = msg[i];
		Ports[M6811_SCCR2] |= M6811_TE;
	        OS_EI();	
	  }

	}
}

void Sporadic (void) {
  char *msg = "Sporadic!"; 
  int i = 0; 
  unsigned int j = 0;
  int len = 9;

  while (1) {
	for (i = 0; i < len; i++) {
		for(j = 1; j != 0; j++);
		OS_DI();
		/* Wait until the SIO has finished to send the character.  */
		while (!(Ports[M6811_SCSR] & M6811_TDRE))
		  continue;
	
		Ports[M6811_SCDR] = msg[i];
		Ports[M6811_SCCR2] |= M6811_TE;
	        OS_EI();	
	  }

	}
}
