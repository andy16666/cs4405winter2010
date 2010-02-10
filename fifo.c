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

fifo_t Fifos[MAXFIFO];

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

void incrementFifoRead(fifo_t *f) {
	circularIncrement(&f->read, FIFOSIZE); 
}

void incrementFifoWrite(fifo_t *f) {
	circularIncrement(&f->write, FIFOSIZE); 
}


