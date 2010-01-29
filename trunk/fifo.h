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
#include <stdlib.h>

typedef struct fifo
{
	FIFO fid;			/* The ID of the FIFO. */
	int elems[FIFOSIZE];
	int nElems;			/* Number of elements currently used in this FIFO */
	int* lastRead;
	int* lastWrite;
} fifo_t;

#endif /* __FIFO_H__ */
