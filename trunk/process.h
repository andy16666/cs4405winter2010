#ifndef __process_h__
#define __process_h__

/*
 * process.h
 * Header file with data types for process management.
 *
 * Authors: 
 * 	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>	
 */
#include "os.h"

/* Maximum time any non-device process can execute in ms. */ 
#define MAX_EXECUTION_TIME 100

#define NEW 0
#define READY 1
#define WAITING 2

typedef volatile unsigned long time_t; 

typedef struct proc_struct {
	PID pid;  	               /* Process ID. */ 
	unsigned int Name;             /* Name of process */ 
	unsigned int Level;            /* Scheduling level/queue */ 
	int   Arg;                     /* Process argument */ 
	char Stack[WORKSPACE]; 
	char *SP;                      /* Last Stack Pointer */ 
	char *ISP;                     /* Initial Stack Pointer */ 
	void(*program_location)(void); /* Pointer to the process, to start it for the first time. */ 

	int state;                     /* NEW, READY, WAITING. */  

	struct proc_struct* Prev;      /* Pointer to the previous process of this process's queue. */ 
	struct proc_struct* Next;      /* Pointer to the next process of the queue. */ 
	
	time_t DevNextRunTime; 	       /* Device process: run next at this time. */ 
} process;

typedef struct kernel_struct {
	char *SP;                      /* Last Stack Pointer */ 
} kernel; 

extern time_t Clock;          /* Software clock, registering the number of miliseconds since system startup. */ 

extern process P[];           /* Main process table.       */ 
extern process *PCurrent;     /* Currently running process */ 
extern process *DevP;         /* Device Process Queue      */ 
extern process *SpoP;         /* Sproatic Process Queue    */
extern process IdleProcess;   /* Pseudo-process to run when ther is nothing else to do. */ 
extern kernel  PKernel;       /* Contains information required to reuturn to kernel mode. */ 

/* Handles tick register overflows. */
void ClockUpdateHandler (void) __attribute__((interrupt)); 

/* Handles the preemption of a process. */ 
void OC4Handler (void) __attribute__((interrupt)); 

/* Updates the software clock from the tick register. */ 
void ClockUpdate(void); 

/* Idle process */ 
void Idle (void); 

/* Perform a context switch to PCurrent */ 
void ContextSwitchToProcess(void); 

/* Perform a context switch to PKernel  */
void ContextSwitchToKernel(void);  

/* Transfer control PCurrent. ONLY used by SWI from ContextSwitch() in kernel mode. */
void SwitchToProcess(void); 

/* Directly return control to kernel. ONLY used by SWI and OS_Terminate(). */ 
void ReturnToKernel(void); 

/* Increment i keeping it within an array of length "max". */ 
void circularIncrement(int *i, int max); 

/* Serch for a periodic process with name 'n' and return a pointer to it. 
   Returns a null pointer if no valid process was found, or if the process is 
   not in the NEW or READY state. */ 
process *GetPeriodicProcessByName(unsigned int n);

/* - Add process to the appropreate scheduling queue, if any. */ 
void AddToSchedulingQueue(process *p); 

/* - Remove process from its scheduling queue, if any. */ 
void RemoveFromSchedulingQueue(process *p); 

/* Add process p to Queue, and return a pointer to the head of the queue. */ 
process *QueueAdd(process *p, process *Queue);

/* Remove process p from Queue and return a pointer to the head of the queue. */ 
process *QueueRemove(process *p, process *Queue);

/* Set OC4 to interrupt at an absolute time, given in miliseconds. */ 
void SetPreemptionTime(time_t time);   

/* Set OC4 to interrupt in a given number of miliseconds. */ 
void SetPreemptionTimerInterval(unsigned int miliseconds); 


#endif /* __process_h__ */
