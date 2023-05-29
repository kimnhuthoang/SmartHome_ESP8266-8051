
#include <REGX52.H>
#include "intrins.h"
#include "i2c_lcd.h"
#include "soft_i2c.h"

//------------------------------------------------------------------------------//
void delay_ms(unsigned int t)
{
	unsigned int i,j;
	for(i=0;i<t;i++)
	{
		for(j=0;j<123;j++);
		_nop_();_nop_();
	}
}
static void delay_us(unsigned int t)
{
	while(t--);
}
//------------------------------------------------------------------------------//
static void lcd_i2c_write(unsigned char Address,unsigned char *dat,unsigned char b)
{
	unsigned char i;

	i2c_start();
	i2c_write(Address);
	for(i=0;i<b;i++) 
	{
		while(i2c_write(dat[i]));
	}
	i2c_stop();
}
//------------------------------------------------------------------------------//
static unsigned char lcd_buff[8];
static unsigned char byte_data;

static void I2C_LCD_Write_4bit(unsigned char dat,unsigned char address);
static void I2C_LCD_SEND_DATA(unsigned char i2c_lcd_address);
void I2C_LCD_WriteCmd(unsigned char cmd,unsigned char address);
//------------------------------------------------------------------------------//
static void I2C_LCD_SEND_DATA(unsigned char i2c_lcd_address)
{
	unsigned char i;
	for (i=0;i<8;i++) 
	{
		byte_data>>=1;
		if(lcd_buff[i]) 
		{
			byte_data|=0x80;
		}
	}
	lcd_i2c_write(i2c_lcd_address,&byte_data,1);
}
//------------------------------------------------------------------------------//
static void I2C_LCD_Write_4bit(unsigned char dat,unsigned char address)
{
	lcd_buff[LCD_D7] = (bit)(dat&0x08);
	lcd_buff[LCD_D6] = (bit)(dat&0x04);
	lcd_buff[LCD_D5] = (bit)(dat&0x02);
	lcd_buff[LCD_D4] = (bit)(dat&0x01);

	lcd_buff[LCD_EN] = 1;
	I2C_LCD_SEND_DATA(address);	
	lcd_buff[LCD_EN] = 0;
	I2C_LCD_SEND_DATA(address);
	
}
//------------------------------------------------------------------------------//
void I2C_LCD_WriteCmd(unsigned char cmd,unsigned char address)
{
	lcd_buff[LCD_RS] = 0;
	I2C_LCD_SEND_DATA(address);
	
	lcd_buff[LCD_RW] = 0;
	I2C_LCD_SEND_DATA(address);
	
	I2C_LCD_Write_4bit(cmd >> 4,address);
	I2C_LCD_Write_4bit(cmd&0x0f,address);
}
//------------------------------------------------------------------------------//
void I2C_LCD_PutC(char cp,unsigned char address)
{
	lcd_buff[LCD_RS] = 1;
	I2C_LCD_SEND_DATA(address);
	
	lcd_buff[LCD_RW] = 0;
	I2C_LCD_SEND_DATA(address);
	
	I2C_LCD_Write_4bit(cp >> 4,address);
	I2C_LCD_Write_4bit(cp&0x0f,address);
	
}
//------------------------------------------------------------------------------//
void I2C_LCD_PutS(char *s,unsigned char address)
{
	while(*s) I2C_LCD_PutC(*s++,address);
}

void I2C_LCD_PutnS(char *s, unsigned char m, unsigned char n,unsigned char address)
{
	unsigned char i;
	s += m;
	for(i = 0; i < n; i++)
	{
		I2C_LCD_PutC(*s,address);
		s++;
	}
}

void I2C_LCD_PutNumber(char num,unsigned char address)
{
	I2C_LCD_PutC(num+0x30,address);
}
//------------------------------------------------------------------------------//
void I2C_LCD_Clear(unsigned char address)
{
	I2C_LCD_WriteCmd(0x01,address);
}
//------------------------------------------------------------------------------//
void I2C_LCD_Gotoxy(unsigned char x,y,adress)
{
	unsigned char address_xy;
	if(y==1) address_xy=(0x80+x-1);
	if(y==2) address_xy=(0xc0+x-1);
	if(y==3) address_xy=(0x94+x-1);
	if(y==4) address_xy=(0xd4+x-1);
	I2C_LCD_WriteCmd(address_xy,adress);
}
//------------------------------------------------------------------------------//
void I2C_LCD_BackLight(unsigned char BackLight,unsigned char address)
{
	if(BackLight) lcd_buff[LCD_BL] = 1;
	else lcd_buff[LCD_BL] = 0;
	I2C_LCD_SEND_DATA(address);
}
//------------------------------------------------------------------------------//
void I2C_LCD_Init(unsigned char address)
{
	unsigned char i;
	delay_ms(20);
	i2c_init();
	for(i=0;i<8;i++){lcd_buff[i]=0;}
	I2C_LCD_SEND_DATA(address);

	I2C_LCD_WriteCmd(0x30,address); delay_ms(5);
	I2C_LCD_WriteCmd(0x30,address); delay_us(10);
	I2C_LCD_WriteCmd(0x30,address); delay_us(4);
	I2C_LCD_WriteCmd(0x02,address); delay_ms(2);
	I2C_LCD_WriteCmd(0x28,address); delay_us(4);
	I2C_LCD_WriteCmd(0x0C,address); delay_us(4);
	I2C_LCD_WriteCmd(0x06,address); delay_us(4);
	I2C_LCD_WriteCmd(0x80,address); delay_us(4);
	I2C_LCD_WriteCmd(0x01,address); delay_ms(2);
}
//------------------------------------------------------------------------------//