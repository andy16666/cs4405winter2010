/*
 * os.c
 * Core operating system kernel code. Designed for the Rug Warrior
 * robot with the HC11 CPU.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 */
 
#include "os.h"
#include "fifo.h"

/* 
  Process Table 
*/
typedef struct proc_str {
	PID pid;  	      /* Process ID. */ 
	unsigned int Name;    /* Name of process */ 
	unsigned int Level;   /* Scheduling level/queue */ 
	int   Arg;            /* Process argument */ 
	short Stack[WORKSPACE]; 
	short *SP;            /* Last Stack Pointer */ 
	short *ISP;           /* Initial Stack Pointer */ 
	interrupt_vectors_t IV; /* Interrupt vector for this process. */ 
	void(*program_location)(void); /* Pointer to the process, to start it for the first time. */ 
	BOOL running;         /* Indicates if the process has been started yet. */ 

	proc_str* QueuePrev;
	proc_str* QueueNext;

	/* Device processes: run next at this time. */ 
	time_t DevNextRunTime; 	
} process;

typedef volatile unsigned long long time_t; 

process P[MAXPROCESS]; /* Main process table. */ 

process *PCurrent; /* Last Process to run */ 
/*process *PNext;*/ /* Next Process to run */

process PKernel;

int PPPLen;   /* Maximum of 16 periodic processes allowed to be in queue */
int PPP[];    /* The queue for periodic scheduling */
int PPPMax[]; /* Maximum CPU time in msec for each process */

process *DevP;         /* Device Process Queue */ 
process *SpoP;         /* Sproatic Process Queue */
 
fifo_t Fifos[MAXFIFO]; /* FIFOs */

time_t Clock;          /* Approximate time since system start in ms. */ 

/* 
  Kernel entry point 
*/
void
main(int argc, char** argv)
{	OS_Init();
	
	PPPLen = 1; 
	PPP    = {IDLE}; 
	PPPMax = {10  }; 

	/* Create processes here */
	
	OS_Start(); 
}
 
/* Initialize the OS */
void
OS_Init()
{	int i; 

	Clock = 0;
	DevP = 0;
	SpoP = 0; 
	PCurrent = 0; 
	
	for (i = 0; i < MAXPROCESS; i++) {
		P[i]->pid = INVALIDPID; 
		P[i]->QueuePrev = 0; 
		P[i]->QueueNext = 0;
		/* Initial stack pointer points at the end of the stack. */ 
		P[i]->ISP = &(P[i]->StackSpace[WORKSPACE-1]); 
	}	
	for (i = 0; i < MAXFIFO; i++) {
		Fifos[i]->fid = INVALIDFIFO; 
	}	
	
	
}
 
/* Actually start the OS */
void
OS_Start()
{
	process *p; 
	time_t t;         /* Time to interrupt. */ 
	int ppp_next;     /* Queue index of the next periodic process. */ 

	ppp_next = 0; 

	while (1) {
		PCurrent = 0;

		p = 0; 
		t = 0;  

		if (DevP) {
			p = DevP; 
			do { 
				if (IsDevPReady(p)) {
					PCurrent = p; 
				}
			} while ((p = DevP->QueueNext()) && (p != DevP)); 
			
			/* If a device process is ready, run it. */ 
			if (PCurrent) {
				PCurrent->DevNextRunTime += (time_t)(PCurrent->Name); 
				asm (" swi "); 
				continue; 
			}
			/* Find the time of the next device process, t. */ 
			else {
				p = DevP; 
				t = p->DevNextRunTime; 
				while ((p = DevP->QueueNext()) && (p != DevP)) { 
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
				PCurrent = GetProcessByName(PPP[ppp_next]); 
			}

			/* Increment ppp_next circularly. */
			ppp_next = (++ppp_next >= PPPLen)?0:ppp_next;

			/* If a periodic process is ready to run, run it. */ 
			if (PCurrent) {
				/* TODO: Set up and OC4 interrupt at time t */ 		
				asm (" swi ");
				continue; 
			}


		}

		/* We're here so we must be idle. Schedule a Sporatic Process. */ 
		/* NOTE: Invalid periodic processes are treated as idle. */ 
		if (SpoP) {
			PCurrent = SpoP; 
			if (t) {
				/* TODO: Set up an OC4 interrupt at time t */
			}
			
			asm (" swi ");
			continue; 
		}
	} 
}
 

BOOL
IsDevPReady(process *p) {
	if (p->DevNextRunTime <= Clock) { return TRUE; } 
	else                            { return FALSE; }
}

process *
GetPeriodicProcessByName(unsigned int n) {
	process *p; 
	for (i = 0; i < MAXPROCESS; i++) {
		p = &P[i]; 
		if (p->pid != INVALIDPID) 
		&& (p->Level == PERIODIC) 
		&& (p->Name == n) {
			return p; 
		}		
	}
	return 0; 
}

void
OS_Abort() {
	/* Kill those lesser peons and then shoot ourselves in the foot */
	asm(" stop "); 
}
 

PID
OS_Create(void (*f)(void), int arg, unsigned int level, unsigned int n) {	
	process *p; 
	int i; 
	
	OS_DI(); 
	/* Find an available process control block */ 
	for (i = 0; i < MAXPROCESS; i++) { 
		p = &P[i];
		if (p->pid == INVALIDPID) {
			p->pid = i+1; 		
			break; 
		}
	} 

	p->Name       = n; 
	p->Level      = level;
	p->Arg        = arg;
	p->running    = FALSE; 	

	p->DevNextRunTime = 0; 

	/* Set the stack pointer to the initial stack pointer. */ 
	p->SP = p->ISP; 
	/* Set the initial program counter to the address of the function representing the process. */ 
	p->program_location = &f;

	/* Add Sporatic Processes to the Sporatic Queue */ 
	if      (p->level == SPORATIC) { SpoP = QueueAdd(p, SpoP); }
	/* Add Device Processes to the Device Queue */ 
	else if (p->level == DEVICE)   { DevP = QueueAdd(p, DevP); }
	
	return p->pid; 
}
 
 

/*
* End a process.
*/
void 
OS_Terminate() {
	OS_DI(); 
	PCurrent->pid = INVALIDPID;

	if      (PCurrent->level == SPORATIC) { SpoP = QueueRemove(PCurrent, SpoP); } 
	else if (PCurrent->level == DEVICE)   { DevP = QueueRemove(PCurrent, DevP); }
	
	/* TODO: Release Semiphores  */
 
	ReturnToKernel(); 
} 

void 
OS_Yield() {
	if (PCurrent->level == SPORATIC) { 
		if (SpoP->QueueNext()) {
			SpoP = SpoP->QueueNext(); 
		}
	} 
	asm(" swi "); 
}



int
OS_GetParam() {
	return PCurrent->Arg; 
}

void 
Idle () {
	while (1); 
}

process *
QueueAdd(process *p, process *Queue) {
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
			Queue->QueueNext = p 
			Queue->QueuePrev = p
		}
	} 
	/* The graph has no existing nodes. */ 
	else { 
		Queue = &p; 
	} 
	return Queue; 
}

process *
QueueRemove(process *p, process *Queue) {
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

/* Increments clock. Run by an interrupt every x ms. */ 
void 
ClockTick(void) {
	OS_DI(); 
	Clock += time_quantum; /* TODO: Calculate time quantum */ 
	OS_EI(); 
}

void 
ReturnToKernel(void)  {
	OS_DI(); 	
	/* TODO: Load Kernel Interrupt Vector */ 
	
	/* Store Process Stack Pointer */
	asm volatile (" sts %0 " : "=m" (PCurrent->SP) : : "memory"); 
	/* Load Kernel Stack Pointer */
	asm volatile (" lds %0 " : : "m" (PKernel.SP) : "memory"); 
	/* Return control to the kernel */ 
	asm volatile (" rti "); 
}

void 
SwitchToProcess(void) {
	/* Store Kernel Stack Pointer */ 
	asm volatile (" sts %0 " : "=m" (PKernel.SP) : : "memory"); 
	/* Load Process Stack Pointer */ 
	asm volatile (" lds %0 " : : "m" (PCurrent->SP) : "memory"); 
	/* TODO: Load Process Interrupt Vector */ 	

	if (PCurrent->running) {		
		/* Return control to running process. */ 
		OS_EI();
		asm volatile (" rti "); 
	} else {
		/* Start process for the first time. */
		PCurrent->running = TRUE; 
		OS_EI();
		PCurrent->program_location(); 
	}
}
