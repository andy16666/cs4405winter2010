/*
 * process.h
 * Header file with data types for process management.
 *
 * Authors: Joel Goguen <r1hh8@unb.ca>
 *			
 */

#ifndef __process_h__
#define __process_h__

#ifndef _OS_H_
#include "os.h"
#endif /* _OS_H_ */

/* Process states */

typedef enum
{
	ready,
	running,
	zombie,
	waiting
} proc_state_t;

/* The idle process */
void idleproc()
{
	while(1);
}

#endif /* __process_h__ */
