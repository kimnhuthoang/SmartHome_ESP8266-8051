
#include <REGX52.H>
#include "soft_i2c.h"

bit i2c_get_ack();
void i2c_ack();
void i2c_nack();

void i2c_init()
{
 	SCL=1; SDA=1;
}

void i2c_start()
{
	SCL = 1; 
	SDA = 1; 
	SDA = 0; 
	SCL = 0;
}

void i2c_stop()
{ 	
	SDA = 0;
	SCL = 1;
	SDA = 1;
}

bit i2c_get_ack() //neu nhan ve 1 bit ACK = 0 la thanh cong
{
	bit result;
  SDA = 1;
  SCL = 1;
	result = SDA;
  SCL = 0;
	return result;
}

bit i2c_write(unsigned char dat)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
	 SDA = (bit)(dat&0x80);	
	 SCL = 1;
	 SCL = 0;
	 dat<<=1;
	}
	return(i2c_get_ack());
}

void i2c_ack()
{
	SDA = 0;
	SCL = 1;
  SCL = 0;
}

void i2c_nack()
{
  SDA = 1;
  SCL = 1;
  SCL = 0;
}

unsigned char i2c_read(bit ack)
{
  unsigned char i, dat=0;
	SDA = 1;
  for(i=0;i<8;i++)
	{
		dat <<= 1;
	  SCL = 1;
		if(SDA)
		{
		 	dat|= 0x01;
		}
		SCL = 0;
  }
	if(ack)i2c_ack();
	else i2c_nack();
  return dat;
}


