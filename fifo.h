/*
 * fifo.h
 * An implementation of a FIFO data structure.  This file contains the 
 * structure definitions and the function prototypes.
 *
 * Authors: 
 *	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>
 */

#ifndef __FIFO_H__
#define __FIFO_H__

#include "os.h"

typedef struct fifo {
	FIFO fid;			/* The ID of the FIFO. */
	int elems[FIFOSIZE];
	int nElems;			/* Number of elements currently used in this FIFO */
	int read;		/* The index of the last element read */
	int write;		/* The index of the last element written */
} fifo_t;

extern fifo_t Fifos[MAXFIFO];

void incrementFifoRead (fifo_t *f); 
void incrementFifoWrite(fifo_t *f); 

#endif /* __FIFO_H__ */
