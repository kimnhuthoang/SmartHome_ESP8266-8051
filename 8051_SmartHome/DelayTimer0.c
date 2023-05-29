
#include <REGX52.H>
#include "DelayTimer0.h"

void T0_delay_us(unsigned int us)		
{
	unsigned int value;
	value = (65535 - us) + 20;
	TH0 = value >> 8;			
	TL0 = value & 0x00ff;	
	TR0 = 1;						
	while(TF0 == 0);		
	TR0 = 0;						
	TF0 = 0;						
}