/*
 * fifo.h
 * An implementation of a FIFO data structure.  This file contains the 
 * structure definitions and the function prototypes.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 */

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "os.h"
#include "process.h"

extern int Semaphores[MAXSEM];
extern process *SemQueues[MAXSEM];

/* - Remove process p from any scheduling queue it is currently in 
   and add it to the waiting queue for semaphore s 
   - Set its state to WAITING */
void MoveToWaitingQueue(process *p, int s); 

/* - Remove the first process (if any) from the waiting queue for semaphore s
   and move it into the appropreate scheduling queue. 
   - Set its state to READY */ 
void MoveNextProcessFromWaitingQueue(int s); 

#endif /* __SEMAPHORE_H__ */
