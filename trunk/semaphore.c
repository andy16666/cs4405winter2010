/*
 * fifo.c
 * An implementation of a FIFO data structure.  This file contains the 
 * functions required for a FIFO to operate.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 */

#include "fifo.h"
#include "os.h"

int Semaphores[MAXSEM];

void OS_InitSem(int s, int n) {
	OS_DI();
	/* Set the semaphore to the number of this resource that are available. */ 
	Semaphores[s] = n; 
	OS_EI(); 
}

void OS_Wait(int s) {
	OS_DI();
	/* While resource is not available, release the CPU */ 
	while (Semaphores[s] <= 0) { OS_Yield(); }
	/* Allocate an instance of the recourse. */ 
	Semaphores[s]--; 
	OS_EI(); 
}

void OS_Signal(int s) {
	OS_DI();
	/* Release resource. */ 
	Semaphores[s]++; 
	OS_EI();
}

