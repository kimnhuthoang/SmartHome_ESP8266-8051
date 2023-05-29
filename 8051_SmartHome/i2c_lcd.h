
#ifndef I2C_LCD_H_
#define I2C_LCD_H_

//#define I2C_LCD_ADDR  0x4E //0x7E =>PCF8574A; 0x4E =>PCF8574//

//#define SCL P0_0
//#define SDA P0_1

#define LCD_RS 0
#define LCD_RW 1
#define LCD_EN 2
#define LCD_BL 3

#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7

void I2C_LCD_Init(unsigned char address);
void I2C_LCD_Gotoxy(unsigned char x,y,adress);
void I2C_LCD_PutC(char cp,unsigned char address);
void I2C_LCD_PutS(char *s,unsigned char address);
void I2C_LCD_PutnS(char *s, unsigned char m, unsigned char n,unsigned char address);
void I2C_LCD_PutNumber(char num,unsigned char address);
void I2C_LCD_Clear(unsigned char address);
void I2C_LCD_BackLight(unsigned char BackLight,unsigned char address);
void I2C_LCD_WriteCmd(unsigned char cmd,unsigned char address);
void delay_ms(unsigned int t);

#endif