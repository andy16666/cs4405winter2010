/*
 * test.h
 *
 * Authors: 
 * 	Joel Goguen <r1hh8@unb.ca>
 *	Andrew Somerville <z19ar@unb.ca>	
 */
#ifndef __TEST_H__
#define __TEST_H__

#define S_BUZZ       15
#define S_BUZZ_FIFO  14
#define S_PORTE      13
#define S_LCD_FIFO   12
#define S_LCD        11
#define S_LOGO       10

#define S_BUZZ_OUTPUT 10

inline void dot(void);
inline void dash(void);

void ProcessInit(void);	

void TestMain(void);

void Buzz(void);
void BuzzString(void);
void FIFOBuzz(void);
void ReadLightSensors(void);
void ReadMicrophone(void);
void ReadBumpers(void);
void PrintBumperValue(void);
void PrintString(void);


void PrintLogo(void);
void Write1(void);
void WriteA(void);
void PrintFIFO(void);

/*****************/
void Test2(void);
void DetectObject(void);
void PrintFIFOInt(void);
/*****************/

/*****************/

#endif
