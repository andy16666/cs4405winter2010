/*
 * fifo.c
 * An implementation of a FIFO data structure.  This file contains the 
 * functions required for a FIFO to operate.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 */


#include "os.h"
#include "semaphore.h"

int Semaphores[MAXSEM];
process *SemQueues[MAXSEM]; 

void OS_InitSem(int s, int n) {
	BOOL I; 
	
	I = CheckInterruptMask(); 
	OS_DI();
	/* Set the semaphore to the number of this resource that are available. */ 
	Semaphores[s] = n; 
	SemQueues[s] = 0; 
	if (!I) { OS_EI(); }
}

void OS_Signal(int s) {
	BOOL I; 
	
	I = CheckInterruptMask(); 
	OS_DI();
	/* Release an instance of this semaphore. */ 
	Semaphores[s]++;
	/* Release the next process from waiting on this semaphore, if any. */ 
	MoveNextProcessFromWaitingQueue(s);
	if (!I) { OS_EI(); }
}

void OS_Wait(int s) { 
	BOOL I; 
	
	I = CheckInterruptMask(); 
	OS_DI();
	/* If resource is not available, move this process into the waiting state, and release the CPU. */ 
	if (Semaphores[s] <= 0) {
		MoveToWaitingQueue(PCurrent,s);	
		OS_Yield();
	}
	/* Allocate an instance of the recourse. */ 
	Semaphores[s]--; 
	if (!I) { OS_EI(); }
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
		p->state = READY; 
		SemQueues[s] = QueueRemove(p,SemQueues[s]); 
		AddToSchedulingQueue(p);
	}
}
