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

extern fifo_t Fifos[];

FIFO
OS_InitFiFo()
{
	int i;
	FIFO id = INVALIDFIFO;
	
	for(i = 0; i < MAXFIFOS; i++)
	{
		if(INVALIDFIFO == Fifos[i].fid)
		{
			id = i;
			Fifos[i].id = i;
			Fifos[i].lastWrite = 0;
			Fifos[i].lastRead = 0;
			Fifos[i].nElems = 0;
			break;
		}
	}
	
	return id;
}

void
OS_Write(FIFO f, int val)
{
	OS_DI();
	
	fifo_t fifo = Fifos[f];
	
	/* Cicularly increment the write counter. */
	fifo.lastWrite = (++(fifo.lastWrite) >= FIFOSIZE) ? 0 : fifo.lastWrite;
	fifo.nElems++;
	
	fifo.elems[fifo.lastWrite] = val;
	
	OS_EI();
}

BOOL
OS_Read(FIFO f, int *val)
{
	OS_DI();
	
	fifo_t fifo = Fifos[f];
	
	/* If there is nothing in the FIFO, fail at reading */
	if(0 == fifo.nElems)
	{
		return FALSE;
	}
	
	/* Circularly increment the read position */
	fifo.lastRead = (++(fifo.lastRead) >= FIFOSIZE) ? 0 : fifo.lastRead;
	*val = fifo.elems[fifo.lastRead];
	fifo.nElems--;
	
	OS_EI();
	return TRUE;
}

