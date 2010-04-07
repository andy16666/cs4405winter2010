/*
 * user.c
 *
 * Authors: 
 * 	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>	
 */
#ifndef __USER_H__
#define __USER_H__

#define S_BUZZ  15
#define S_PORTE 14

void ProcessInit(void);	

/*****************/
void Test2(void);
void ReadLightSensors(void);
void DetectObject(void);
void PrintFIFOInt(void);
/*****************/ 
void Buzz(void);
/*****************/
void Test1(void);

void Write1(void);
void WriteA(void);
void PrintFIFO(void);
/*****************/

void PrintTime (void); 

#endif
