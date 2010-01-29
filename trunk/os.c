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
  Process Table 
*/
typedef struct proc_str {
	PID PID;  	     /* Process ID. */ 
	unsigned int Name;   /* Name of process */ 
	unsigned int Level;  /* Scheduling level/queue */ 
	int Arg;             /* Process argument */ 
	char *SP; 
	void(*PC)(void);
	proc_str* QueuePrev;
	proc_str* QueueNext;
	proc_str* Prev;
	proc_str* Next;
} process;

process P[MAXPROCESS];        /* Main process table. */ 

process *PLast; /* Last Process to run */ 
process *PNext; /* Next Process to run */

/*
  Periodic Process Queue
*/ 
int PPPLen;   /* Maximum of 16 periodic processes allowed to be in queue */
int PPP[];    /* The queue for periodic scheduling */
int PPPMax[]; /* Maximum CPU time in msec for each process */

/*
  Device Process Queue 
*/ 
process *DevP; 

/* 
  Sproatic Process Queue
*/ 
process *SpoP;

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

	/* Create processes here */
	
	OS_Start(); 
}
 
/* Initialize the OS */
void
OS_Init()
{	int i; 

	SpoPNext  = 0; 
	PLast     = INVALIDPID; 
	PNext     = INVALIDPID; 
	
	for (i = 0; i < MAXPROCESS; i++) {
		P[i]->PID = INVALIDPID; 
	}	
	DevP = 0;
	SpoP = 0;
}
 
/* Actually start the OS */
void
OS_Start()
{

	while(1) {
		/* Check queues and find the next process to run. */ 
	
		

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
{	process *p; 
	int i; 
	
	/* Find an available process control block */ 
	for (i = 0; i < MAXPROCESS; i++) { 
		p = &P[i];
		if (p->PID == INVALIDPID) {
			p->PID = i+1; 		
			break; 
		}
	} 

	p->Name       = n; 
	p->Level      = level;
	p->Arg        = arg;
	
	/* Set the stack pointer to the last address in the workspace. */ 
	p->SP = (char*)((&StackSpace)+(WORKSPACE*(p->PID))-1); 
	/* Set the initial program counter to the address of the function representing the process. */ 
	p->PC = &f;

	/* Add Sporatic Processes to the Sporatic Queue */ 
	if (p->level == SPORATIC) {
		SpoP[p->PID-1] = &p; 
	}
	/* Add Device Processes to the Device Queue */ 
	else if (p->level == DEVICE) {
		DevP[p->PID-1] = &p; 
	}
	
	return p->PID; 
}
 
/*
* End a process.
*/
void 
OS_Terminate() {
	PLast->PID = INVALIDPID;

	/* Remove from queues */
	
	OS_Yield();
} 

int
OS_GetParam() {
	return PLast->Arg; 
}

void 
Idle () {
	while (1); 
}
