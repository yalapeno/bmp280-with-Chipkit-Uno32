#include <stdint.h>
#include <pic32mx.h>
void *stdin, *stdout, *stderr;
//#include "mipslab.h"
//we want
//
//

//
int getsw(void) 
{ 	
	return (PORTD >> 8) & 0xf;
}
int getbtns (void)
{
	return (PORTD >> 5) & 0x7; // 111 = 7
}
