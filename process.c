/*
 * process.c
 * Header file with data types for process management.
 *
 * Authors: 
 * 	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>	
 */
#include "process.h"

int PPPLen;
int PPP[MAXPROCESS]; 
int PPPMax[MAXPROCESS];

process P[MAXPROCESS]; /* Main process table.        */ 
process *PCurrent;     /* Currently running process. */ 
process *DevP;         /* Device Process Queue       */ 
process *SpoP;         /* Sproatic Process Queue     */

process IdleProcess; 
kernel  PKernel;

time_t Clock;       /* Approximate time since system start in ms. */ 

void UnhandledInterrupt (void) { return; }  

void Idle (void) { while (1); }

/* Preemption must be handled differently from traps to preserve local variables. */ 
void OC4Handler(void) { ContextSwitchToKernel(); }

void circularIncrement(int *i, int max) {
        *i = (++(*i) >= max)?0:*i;
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
        //Ports[M6811_TCTL1] SET_BIT(M6811_BIT2);
		 
		/* Set OC4F */  
        Ports[M6811_TFLG1] SET_BIT(M6811_BIT4);
        /* Unmask OC4 interrupt */
        Ports[M6811_TMSK1] SET_BIT(M6811_BIT4);
}

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
                elapsed_time = (0xFFFF - last_timer_value) + timer_value; 
                Ports[M6811_TFLG2] CLR_BIT(M6811_BIT7);
        }
		else {
                elapsed_time = timer_value - last_timer_value;
        }

        /* Add on the residual from the last update. */ 
        elapsed_time += residual; 

        residual = elapsed_time % TIME_QUANTUM; 

        Clock += (elapsed_time-residual)/TIME_QUANTUM;

        /* Account for the residual during the next update. */ 
        last_timer_value = timer_value; 
}

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

void ReturnToKernel(void)  {
        OS_DI();        
        /* Store the stack pointer in the given location. */ 
        asm volatile (" sts %0 " : "=m" (PCurrent->SP) : : "memory"); 
        /* Correct for function call. */
        PCurrent->SP++; 
        PCurrent->SP++;
        
		/* Clear OC4 Flag */  
        Ports[M6811_TFLG1] CLR_BIT(M6811_BIT4);
		/* Mask OC4 interrupts */
        Ports[M6811_TMSK1] CLR_BIT(M6811_BIT4);
		
        /* Reset interrupt handlers. */ 
        IVOC4 = UnhandledInterrupt; /* The kernel cannot be preempted. */ 
        IVSWI = SwitchToProcess; 

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
        IVOC4 = OC4Handler; /* OC4 must jump out to a proper interrupt handler to preserve local variables. */ 
        IVSWI = ReturnToKernel;

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
				/* Something went wrong...we shouldn't be here...return to kernel. */ 
                asm volatile (" lds %0 " : : "m" (PKernel.SP) : "memory"); 
                asm volatile (" rti ");
        }
}

BOOL CheckInterruptMask () {
	/* Non-zero if interrupts were previously masked. */ 
	unsigned char CC; 
	
	asm volatile ("tpa \n\t staa %0 " : "=m" (CC) : : "a","memory"); 
	
	if (CC & M6811_BIT4) {
		return TRUE; 
	}
	else {
		return FALSE; 
	}
}
