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
			break;
		}
	}
	
	return id;
}

void
OS_Write(FIFO f, int val)
{
	OS_DI();
	
	
	
	OS_EI();
}
