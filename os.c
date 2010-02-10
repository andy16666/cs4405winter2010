/*
 * os.c
 * Core operating system kernel code. Designed for the Rug Warrior
 * robot with the HC11 CPU.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 *
 * m68hc11-gcc -g -mshort -Wl,-m,m68hc11elfb  -msoft-reg-count=0 os.c -o os.elf
 */
#include "os.h"
#include "process.h"
#include "fifo.h" 
#include "semaphore.h"
#include "ports.h"
#include "interrupts.h"
#include "interrupts.c"


#define M6811_CPU_KHZ 400000
#define TIME_QUANTUM M6811_CPU_KHZ/16

/* Processes */ 
void PrintChar(void);	
void PrintString(void);
void PrintInit (void); 
void PrintTime (void); 
void PrintSystemTime (void);

fifo_t Fifos[MAXFIFO]; /* FIFOs */

int Semaphores[MAXSEM];
process *SemQueues[MAXSEM]; 

process P[MAXPROCESS]; /* Main process table.        */ 
process *PCurrent;     /* Currently running process. */ 
process *DevP;         /* Device Process Queue       */ 
process *SpoP;         /* Sproatic Process Queue     */

process IdleProcess; 
kernel  PKernel;

int PPPLen;
int PPP[MAXPROCESS]; 
int PPPMax[MAXPROCESS];

time_t Clock;       /* Approximate time since system start in ms. */ 

void Reset (void) { 
	unsigned int i; 
	/* Set the timer prescale factor to 16 (1,1)...Must be set very soon after startup!!! */
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT0);
	Ports[M6811_TMSK2] SET_BIT(M6811_BIT1);

	/* Wait a few thousand cycles for the cpu timing to settle. */   
	for (i = 1; i !=0; i++); 

	
	main(); 
}

int main() {
	unsigned int i; 
	char *test = "\r                     ABCDEFGHIJKLMNOPQRSTUVWXYZ\r"; 

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

	IV.SWI = SwitchToProcess; 

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

FIFO OS_InitFiFo() {
	int i;
	FIFO id = INVALIDFIFO;
	
	for(i = 0; i < MAXFIFO; i++) {
		if(INVALIDFIFO == Fifos[i].fid)	{
			Fifos[i].fid       = id = i + 1;
			Fifos[i].write = 0;
			Fifos[i].read  = 0;
			Fifos[i].nElems    = 0;
			break;
		}
	}
	return id;
}

void OS_Write(FIFO f, int val) {	
	fifo_t *fifo;
	
	fifo = &Fifos[f];

	OS_DI();
	fifo->elems[fifo->write] = val;
	
	/* Increment the write counter. */
	incrementFifoWrite(fifo); 

	if(fifo->nElems >= FIFOSIZE) { incrementFifoRead(fifo); }
	else                         { fifo->nElems++; }

	OS_EI();
}

void OS_InitSem(int s, int n) {
	OS_DI();
	/* Set the semaphore to the number of this resource that are available. */ 
	Semaphores[s] = n; 
	OS_EI(); 
}

void OS_Wait(int s) { 
	OS_DI();
	/* If resource is not available, move this process into the waiting state, and release the CPU. */ 
	if (Semaphores[s] <= 0) {
		MoveToWaitingQueue(PCurrent,s);
		OS_Yield();  
	}
	/* Allocate an instance of the recourse. */ 
	Semaphores[s]--; 
	OS_EI(); 
}

void OS_Signal(int s) {
	OS_DI();
	/* Release an instance of this semaphore. */ 
	Semaphores[s]++; 
	/* Release the next process from waiting on this semaphore, if any. */ 
	MoveNextProcessFromWaitingQueue(s);
	OS_EI();
}


BOOL OS_Read(FIFO f, int *val) {
	fifo_t *fifo = &Fifos[f];
	
	/* If there is nothing in the FIFO, fail at reading */
	if(0 == fifo->nElems) {
		return FALSE;
	}
	
	OS_DI();
	*val = fifo->elems[fifo->read];
	/* Circularly increment the read position */	
	incrementFifoRead(fifo);	
	fifo->nElems--;	
	OS_EI();
	return TRUE;
}

void MoveToWaitingQueue(process *p, int s) {
	p->state = WAITING; 
	RemoveFromSchedulingQueue(p); 
	SemQueues[s] = QueueAdd(p,SemQueues[s]); 
}

void MoveNextProcessFromWaitingQueue(int s) {
	process *p; 
	/* Remove the first process from the queue for the given semaphore, and make it ready. */ 
	if (p = SemQueues[s]) {
		SemQueues[s] = QueueRemove(p,SemQueues[s]); 
		AddToSchedulingQueue(p); 
		p->state = READY; 
	}
}

void AddToSchedulingQueue(process *p) {
	/* Add Sporatic Processes to the Sporatic Queue */ 
	if (p->Level == SPORADIC) { SpoP = QueueAdd(p, SpoP); }
	/* Add Device Processes to the Device Queue */ 
	if (p->Level == DEVICE)   { DevP = QueueAdd(p, DevP); }
}

void RemoveFromSchedulingQueue(process *p) {
	/* Remove the process from the SPORATIC queue */
	if (p->Level == SPORADIC) { SpoP = QueueRemove(p, SpoP); } 
	/* Remove the process from the DEVICE queue */
	if (p->Level == DEVICE)   { DevP = QueueRemove(p, DevP); }
}

void SetPreemptionTime(time_t time) {
	SetPreemptionTimerInterval((unsigned int)(time - Clock)); 
}

void SetPreemptionTimerInterval(unsigned int miliseconds) {
	unsigned int *TOC4_address; 
	unsigned int *timer_address; 
	
	/* Make sure these are read as signel 16 bit numbers. */ 
	TOC4_address  = (unsigned int*)&(Ports[M6811_TOC4_HIGH]);
	timer_address = (unsigned int*)&(Ports[M6811_TCNT_HIGH]);

	/* Read the tick register, add a time to it, and store it in the OC4 compare register. */ 
	*TOC4_address = (*timer_address + (miliseconds * TIME_QUANTUM)); 
	/* Set OL4 */ 
	Ports[M6811_TCTL1] SET_BIT(M6811_BIT2);
	/* Set OC4F */ 	
	Ports[M6811_TFLG1] SET_BIT(M6811_BIT4);
	/* Unmask OC4 interrupt */
	Ports[M6811_TMSK1] SET_BIT(M6811_BIT4);
}

void ContextSwitchToKernel(void)  { 
	OS_DI(); 
	/* Interrupt to a SwitchToProcess() or ReturnToKernel() ISRs. */ 
	asm volatile (" swi "); 
	/* This function returns when rti is called. */
	OS_EI();
}

void ContextSwitchToProcess(void) { 
	/* Interrupt to a SwitchToProcess() or ReturnToKernel() ISRs. */ 
	asm volatile (" swi "); 
	/* This function returns when rti is called. */
}

void UnhandledInterrupt (void) { return; }  


/* Interrupt service routine for updating the clock. */ 
void ClockUpdateHandler (void) { 
	ClockUpdate(); 
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

	residual = elapsed_time % TIME_QUANTUM; 

	Clock += (elapsed_time-residual)/TIME_QUANTUM; 

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
		&& (PCurrent->state != WAITING) 
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
		if (Queue->Next && Queue->Prev) {
			p->Prev = Queue->Prev; 
			p->Next = Queue; 
			Queue->Prev->Next = p; 
			Queue->Prev = p; 
		} 
		/* The graph has exactly one existing node. */ 
		else {
			p->Next = Queue; 
			p->Prev = Queue;
			Queue->Next = p; 
			Queue->Prev = p;			
		}
	} 
	/* The graph has no existing nodes. */ 
	else { 
		p->Next = 0; 
		p->Prev = 0;
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
			if (Queue->Next) { Queue = Queue->Next; }
			/* There is only one node and it is the one to be removed. */ 
			else                  { return 0; }
		}
		/* The graph has only two nodes  */ 
		if (p->Prev == p->Next) {
			Queue->Prev = 0; 
			Queue->Next = 0;
		}
		/* The graph has more than two nodes. */ 
		else {
			p->Prev->Next = p->Next; 
			p->Next->Prev = p->Prev; 
		}
	}
	
	p->Next = 0; 
	p->Prev = 0;
 
	return Queue; 
}

/* Preemption must be handled differently from traps to preserve local variables. */ 
void OC4Handler(void) {
	asm(" swi ");
}


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
	IV.OC4 = UnhandledInterrupt; /* The kernel cannot be preempted. */ 
	IV.SWI = SwitchToProcess; 

	/* Load Kernel Stack Pointer */
	asm volatile (" lds %0 " : : "m" (PKernel.SP) : "memory"); 
	/* Return control to the kernel */ 
	asm volatile (" rti "); 
}
 
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
	if (PCurrent->state == READY) {		
		/* Load Process Stack Pointer */ 
		asm volatile (" lds %0 " : : "m" (PCurrent->SP) : "memory"); 
		/* Return control to running process. */ 
		asm volatile (" rti "); 
	} 
	/* If the process has not been started, we need to start it for the first time. */ 
	else if (PCurrent->state == NEW) {
		/* Set the process to the ready state. */
		PCurrent->state = READY; 
		/* Load Process Stack Pointer */ 
		asm volatile (" lds %0 " : : "m" (PCurrent->ISP) : "memory"); 
		/* Run process for the first time. */
		OS_EI();
		PCurrent->program_location();
		OS_DI();
		/* When the process ends run terminate to clean it up. */ 
		OS_Terminate(); 
	}
	else {
		OS_Abort(); 
	}
}

void circularIncrement(int *i, int max) {
	*i = (++(*i) >= max)?0:*i;
}

void incrementFifoRead(fifo_t *f) {
	circularIncrement(&f->read, FIFOSIZE); 
}

void incrementFifoWrite(fifo_t *f) {
	circularIncrement(&f->write, FIFOSIZE); 
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



