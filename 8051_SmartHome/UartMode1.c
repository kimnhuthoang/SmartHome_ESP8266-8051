//thu vien giao tiep UART Kim Nhut Hoang 10DHDT1

#include <REGX52.H>
#include "UartMode1.h"

void uart_init(){    //khoi tao giao tiep uart
	TMOD |= 0x20;       // timer 1 che do 2 tu dong nap lai 8 bit
	TH1  = 0xFD;				 // nap gia tri toc do baud 9600 thach anh 11.0592 MHz
	SCON = 0x50;            
	TR1  = 1;
}
void uart_isrInit()
{
	EA = 1; ES = 1;
}
void uart_write(char c){ //gui 1 ky tu ra
	SBUF = c;
	while(TI==0);
	TI = 0;
}
void uart_write_text(char *str){ // gui chuoi ky tu ra
	unsigned char i=0;
	ES = 0;
	while(str[i]!='\0'){
		uart_write(str[i]);
		i++;
	}	
	ES = 1;
}
void uart_write_number(char num)
{
	uart_write(num+0x30);
}

char uart_data_ready(){return RI;}   //san sang nhan data

unsigned char uart_read() //nhan va doc data tu sbuf
{
	unsigned char byte=0;
	if(RI==1)
	{
		RI=0; 
		byte = SBUF;
		return byte;
	}
	else 
	{
		return 0;
	}
} 
