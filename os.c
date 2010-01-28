/*
* os.c
* Core operating system kernel code. Designed for the Rug Warrior
* robot with the HC11 CPU.
*
* Authors: 
*	Joel Goguen <r1hh8@unb.ca>
*	Andrew Somerville <z19ar@unb.ca>
*/
 
#include "os.h"

/*
  Periodic Process Queue
*/ 
/* Maximum of 16 periodic processes allowed to be in queue */
int PPPLen;
/* The queue for periodic scheduling */
int PPP[];
/* Maximum CPU time in msec for each process */
int PPPMax[];

/*
  Device Process Queue 
*/ 
PID DevP[MAXPROCESS]; 
unsigned int DevPCount; 
unsigned int DevPRate[MAXPROCESS]; 

/* 
  Sproatic Process Queue
*/ 
PID SpoP[MAXPROCESS]
unsigned int SpoPCount; 
unsigned int SpoPNext; 

/* 
  Process Table 
*/
unsigned int PCount;               /* Current number of processes in the system. */
unsigned int PName  [MAXPROCESS];  /* Names of processes */ 
unsigned int PLevel [MAXPROCESS];  /* Scheduling levels of processes */ 
int PArg [MAXPROCESS];             /* Arguments passed to processes */ 
BOOL PTerminated[MAXPROCESS]; 
char *PSP [MAXPROCESS]; 
void(*PPC [MAXPROCESS])(void); 


PID PLast; /* Last Process to run */ 
PID PNext; /* Next Process to run */

/* Stack Space */
char StackSpace[MAXPROCESS*WORKSPACE]; 
/* FIFOs */
char FIFOs[MAXFIFO][FIFOSIZE]; 


 
/* 
  Kernel entry point 
*/
void
main(int argc, char** argv)
{
	OS_Init();
	
	PPPLen = 1; 
	PPP    = {IDLE}; 
	PPPMax = {10  }; 

	OS_Start(); 
}
 
/* Initialize the OS */
void
OS_Init()
{
	PCount    = 0; 
	DevPCount = 0; 
	SpoPCount = 0; 
	SpoPNext  = 0; 
	PLast     = INVALIDPID; 
	PNext     = INVALIDPID; 
}
 
/* Actually start the OS */
void
OS_Start()
{

	while(1) {
		/* Schedule processes */
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
* - f: Pointer to the function to execute.
* - arg: Initial argument for f, optionally used, retrieved by calling OS_GetParam().
* - level: The scheduling level for this process.
* - n: The "name" of this process. For device processes, n represents the 
*
* Returns:
* The PID of the new process.
*/
PID
OS_Create(void (*f)(void), int arg, unsigned int level, unsigned int n)
{
	PID pid; 

	pid = PCount+1; 

	PName       [pid-1] = n; 
	PLevel      [pid-1] = level;
	PArg        [pid-1] = arg;
	PTerminated [pid-1] = FALSE; 
	
	/* Set the stack pointer to the last address in the workspace. */ 
	PSP [pid-1]   = (char*)((&StackSpace)+(WORKSPACE*pid)-1); 
	PPC [pid-1]   = &f;

	/* Add Sporatic Processes to the Sporatic Queue */ 
	if (level == SPORATIC) {
		SpoP[SpoPCount] = pid; 
		SpoPCount++; 
	}
	/* Add Device Processes to the Device Queue */ 
	else if (level == DEVICE) {
		DevP[DevPCount] = pid; 
		DevPRate[DevPCount] = n; 
		DevPCount++; 
	}
	PCount++; 
}
 
/*
* End a process.
*/
void 
OS_Terminate() {
	PTerminated[PLast-1] = TRUE; 	
} 

void 
OS_GetParam() {
	return PArg[PLast-1]; 
}

void 
Idle () {
	while (1); 
}
