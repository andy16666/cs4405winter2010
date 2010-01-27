/*
 * os.c
 * Core operating system kernel code.  Designed for the Rug Warrior 
 * robot with the HC11 CPU.
 *
 * Authors: Joel Goguen <r1hh8@unb.ca>
 *			
 */

#include "os.h"

/* Maximum of 16 periodic processes allowed to be in queue */
int PPPLen = 0;

/* The queue for periodic scheduling */
int PPP[];

/* Maximum CPU time in msec for each process */
int PPPMax[];

/* Process Table */ 
unsigned int PNum; 
unsigned int PName[MAXPROCESS];
unsigned int PSP[MAXPROCESS];
unsigned int PLevel[MAXPROCESS];
unsigned int PPC[MAXPROCESS];

char StackSpace[MAXPROCESS * WORKSPACE];

/* Have we been initialized? */
BOOL init_done = FALSE;
BOOL start_done = FALSE;

/* Kernel entry point */
void
main(int argc, char** argv)
{
	/* Initialize the OS so we can actually "Do Stuff" */
	OS_Init();
}

/* Initialize the OS */
void
OS_Init()
{
	/* Only allow init once */
	if(FALSE == init_done)
	{
		PNum = 0;

		/* Sacrifice 3 goats and a virgin camel to initialize the OS */

		/* Flag that we've made the requisite sacrifices and appeased the gods */
		init_done = TRUE;
	}
}

/* Actually start the OS */
void
OS_Start()
{
	/* Make sure not to run again if this has already been done */
	if(FALSE == start_done)
	{
		/* Perform black magic required to actually start the OS */
		
		/* Black magic vials empty, no more virgin camels, don't allow calling again */
		start_done = TRUE;
	}
}

void
OS_Abort()
{
	/* Kill those lesser peons and then shoot ourselves in the foot */
}

/* 
 * Create a process.
 * 
 * Parameters:
 *		- f: Pointer to the function to execute.
 *		- arg: Initial argument for f, optionally used, retrieved by calling OS_GetParam().
 *		- level: The scheduling level for this process.
 *		- n: The "name" of this process.
 *
 * Returns:
 *		The PID of the new process.
 */
PID
OS_Create(void (*f)(void), int arg, unsigned int level, unsigned int n)
{
	/* Well, when a kernel and a process really love each other... */
	PName[PNum] = n;
	PSP[PNum] = (&StackSpace) + (WORKSPACE + (PNum + 1));

	PNum++;
}

/*
 * End a process.
 */
