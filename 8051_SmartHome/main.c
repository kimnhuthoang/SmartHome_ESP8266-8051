 
/*Duoc viet boi Kim Nhut Hoang - 2002190238 - 10DHDT1*/ 
/*Su dung cac thu vien tu viet va cac thu vien tham khao tren internet
	cua cac dong vi dieu khien khac tu sua lai de co the chay tren vi dieu
	khien 8051
*/
//----------------------------------------------------------------
//---Khai bao cac thu vien su dung---//
#include <REGX52.H>
#include <intrins.h>
#include "soft_i2c.h"
#include "i2c_lcd.h"
#include "eeprom_24Cxx.h"
#include "mfrc522.h"
#include "UartMode1.h"
#include "DelayTimer0.h"
//----------------------------------------------------------------
//---Khai bao va gan cac bien---//
#define DEVICE_STATE 250

#define Den1_ON   0xfd
#define Den2_ON   0xfb
#define Den3_ON   0xf7
#define Quat1_ON  0xef
#define Quat2_ON  0xdf
#define Quat3_ON  0xbf

#define Den1_OFF   0x02
#define Den2_OFF   0x04
#define Den3_OFF   0x08
#define Quat1_OFF  0x10
#define Quat2_OFF  0x20
#define Quat3_OFF  0x40

#define IE74595_DS    P0_3
#define IE74595_STCP  P0_2
#define IE74595_SHCP  P0_1

#define cot1   P1_7
#define cot2   P1_6
#define cot3   P1_5
#define hang1  P1_4
#define hang2  P1_3
#define hang3  P1_2
#define hang4  P1_1

#define Reset  P0_7
#define Button_Open P3_7

#define led_connect P0_0
#define Power 			P0_5
#define Reset_ESP 	P1_0
#define buzzer P2_0
#define door   P0_4

#define But_Den1   P3_2
#define But_Den2   P3_3
#define But_Den3   P3_4
#define But_Quat1  P3_5
#define But_Quat2  P3_6
//#define But_Quat3  P3_7

unsigned char Buffer[18];
unsigned char idata Buffer_last[18];
bit uart_isr_flag = 0;
bit power_off = 1;
unsigned char dataIE74595 = 0xff;

char x = 0, y = 0, wrong_time = 0, CPin_flag = 0;
unsigned char pin_default[4]={"1234"};
unsigned char pin_scan[4]; 
unsigned char pin_eeprom[4]; 

unsigned char UID_Scan[4];
unsigned char UID_EEPROM[4];
//unsigned char RemoveCardValue[4]={0,0,0,0};
unsigned char Master_id[4]={0xe3,0x03,0x21,0x97};

char check_id = 0; check_masterCard = 2; check_newcard = 0;
unsigned char open_menu = 0,select_menu = 0,enter_select = 0;
char AddrZero = 5; CardYes = 0; CardNo = 0; CardRemoveAddr = 5;

//--------------------------------------------------------------
void show_main(); //hien thi man hinh chinh
void Write_eeprom(unsigned char *str,unsigned int address); //ghi 4byte du lieu vao EEPROM
void Read_eeprom(unsigned char *str,unsigned int address);  //Doc 4byte du lieu tu EEPROM
void IE74595_OutByte(unsigned char b); //xuat du lieu ra ic mo rong 74hc595
void clear_buff();  //Xoa mang dem
void copy_buff();
void Uart_handle(); //Ham xu li chuoi nhan tu UART
void Button_OnOff_Device(); //Nut nhan bat tat thiet bi
void servo_open();
void servo_close();
//--------------------------------------------------------------
// Keypad function
char Keypad(); // Ham quet ban phim
char Keypad_changePin(); //Ham quet phim khi doi Pass
void Check_Pin();  //Ham kiem tra Pass
void Change_Pin(); //Ham doi Pass
void CheckOldPin_and_ChangePin(); //Ham kiem tra Pass cu va doi Pass moi
void ScanKeyRFID_and_Check(); //Ham quet ban phim va quet the RFID va kiem tra
void buzzer_ring(); //Ham tao am thanh cho coi Buzzer
void Button_OpenDoor();
void open_and_closed(); //Ham mo va dong cua
void Reset_pin(); //Ham reset mat khau mac dinh
//--------------------------------------------------------------
// RFID function
void Scan_RFID(); //Ham Scan the RFID
char Check_Card(char UID_Scan[],char Data_Card[]); //Ham kiem tra the RFID
void Check_Master(); //Ham kiem tra the Master
void Menu_RFID(); //Ham menu RFID
void Menu_RFID_All(); //Ham menu RFID
void Menu_RFID_Select(); //Ham menu RFID
void Compare_EEPROM(); //Ham so sanh du lieu luu trong EEPROM
void FindEEPROM_Zero(); //Ham tim dia chi ko co du lieu trong EEPROM
void addNew_Card(); //Ham them the RFID moi
void Remove_Card(); //Ham xoa the RFID
//void Reset_MasterCard();
//--------------------------------------------------------------

void main()
{
	Reset_ESP = 0; delay_ms(20); Reset_ESP = 1;
	door = 0; IE74595_STCP = 0; IE74595_SHCP = 0;
	TMOD |= 0x01; //Timer 0 che do 16bit
	uart_init(); uart_isrInit();
	EEPROM_AT24Cxx_init(0xA0,8);
	dataIE74595 = EEPROM_AT24Cxx_read(DEVICE_STATE);
	IE74595_OutByte(dataIE74595);

	MFRC522_Init();
	I2C_LCD_Init(0x4e);
	I2C_LCD_BackLight(1,0x4e);
	I2C_LCD_Init(0x4c);
	I2C_LCD_BackLight(1,0x4c);
	servo_close();

	delay_ms(10);
	I2C_LCD_Clear(0x4e);
	I2C_LCD_Gotoxy(2,1,0x4e); I2C_LCD_PutS("KIM NHUT HOANG",0x4e);
	I2C_LCD_Gotoxy(4,2,0x4e); I2C_LCD_PutS("2002190238",0x4e);
	
	I2C_LCD_Clear(0x4c);
	I2C_LCD_Gotoxy(3,1,0x4c);
	I2C_LCD_PutS("TIME: 00:00",0x4c);
	I2C_LCD_Gotoxy(1,2,0x4c);
	I2C_LCD_PutS(" T: 00",0x4c);I2C_LCD_PutC(0xdf,0x4c);I2C_LCD_PutC('C',0x4c);
	I2C_LCD_PutS("-H: 00%",0x4c);
	
	delay_ms(2000);
	show_main();
	while(1)
	{
		ScanKeyRFID_and_Check();
	}
}
//-------------------------------------------------------------------------

void uart_isrfunction(void) interrupt 4 using 3
{
	static unsigned char Str_len = 0;
	if(RI == 1)
	{
		Buffer[Str_len++] = SBUF;
		RI = 0;
		if(SBUF=='\n')
		{
			Str_len = 0; uart_isr_flag = 1;
//			if(Buffer[0]=='S')
//			{
//				if(Buffer[1]=='1') led_connect = 0;
//				else led_connect = 1;
//				uart_isr_flag = 0;
//				clear_buff();
//			}
		}
	}
}

void Uart_handle()
{
	if(uart_isr_flag == 1)
	{
		uart_isr_flag = 0;
		copy_buff();
//		clear_buff();
//		ES = 0;
		if(Buffer_last[0]=='S')
		{
			if(Buffer_last[1]=='1') led_connect = 0;
			else led_connect = 1;
		}
		//---------------------------------------------------------------------------//
		else if(Buffer_last[0]=='d')
		{
			//------------------------------------------------------------------------//
																	//DEN 1//
			if(Buffer_last[0]=='d' && Buffer_last[1]=='1')
			{ 
				if(Buffer_last[2]=='1') IE74595_OutByte(dataIE74595&=Den1_ON);
				else IE74595_OutByte(dataIE74595|=Den1_OFF);
			}
			//------------------------------------------------------------------------//
																		//DEN 2//
			if(Buffer_last[3]=='d' && Buffer_last[4]=='2')
			{ 
				if(Buffer_last[5]=='1') IE74595_OutByte(dataIE74595&=Den2_ON);
				else IE74595_OutByte(dataIE74595|=Den2_OFF);
			}
			//------------------------------------------------------------------------//
																		//DEN 3//
			if(Buffer_last[6]=='d' && Buffer_last[7]=='3')
			{ 
				if(Buffer_last[8]=='1') IE74595_OutByte(dataIE74595&=Den3_ON);
				else IE74595_OutByte(dataIE74595|=Den3_OFF);
			}
			//------------------------------------------------------------------------//
																		//QUAT 1//
			if(Buffer_last[9]=='q' && Buffer_last[10]=='1')
			{ 
				if(Buffer_last[11]=='1') IE74595_OutByte(dataIE74595&=Quat1_ON);
				else IE74595_OutByte(dataIE74595|=Quat1_OFF);
			}
			//------------------------------------------------------------------------//
																		//QUAT 2//
			if(Buffer_last[12]=='q' && Buffer_last[13]=='2')
			{ 
				if(Buffer_last[14]=='1') IE74595_OutByte(dataIE74595&=Quat2_ON);
				else IE74595_OutByte(dataIE74595|=Quat2_OFF);
			}
			//------------------------------------------------------------------------//
																		//QUAT 3//
			if(Buffer_last[15]=='q' && Buffer_last[16]=='3')
			{ 
				if(Buffer_last[17]=='1') IE74595_OutByte(dataIE74595&=Quat3_ON);
				else IE74595_OutByte(dataIE74595|=Quat3_OFF);
			}
			//------------------------------------------------------------------------//
													//LUU TRANG THAI VAO EEPROM//
			EEPROM_AT24Cxx_write(DEVICE_STATE,dataIE74595); delay_ms(20);
		}
		
		//------------------------------------------------------------------------//
		else
		{
											//NHAN DU LIEU NHIET DO, DO AM VA THOI GIAN//
			if(Buffer_last[0]=='n' && Buffer_last[1]=='d')
			{
				I2C_LCD_Gotoxy(5,2,0x4c); I2C_LCD_PutnS(Buffer_last,2,2,0x4c);
			}
			if(Buffer_last[4]=='d' && Buffer_last[5]=='a')
			{
				I2C_LCD_Gotoxy(13,2,0x4c); I2C_LCD_PutnS(Buffer_last,6,2,0x4c);
			}
			if(Buffer_last[8]=='T')
			{
				I2C_LCD_Gotoxy(9 ,1,0x4c); I2C_LCD_PutnS(Buffer_last,9,5,0x4c);
			}
		}
		//------------------------------------------------------------------------//
//		ES = 1;
	}	
}

void clear_buff()
{
	char j;
	for(j = 0; j < 18; j++)
	{
		Buffer[j]='\0';
	}
}

void copy_buff()
{
	char i;
	for(i = 0; i < 18; i++)
	{
		Buffer_last[i] = Buffer[i];
	}
}
//-------------------------------------------------------------------------
void Button_OnOff_Device()
{
	if(But_Den1 == 0)
	{
		while(But_Den1 == 0);
		if(dataIE74595&Den1_OFF){uart_write_text("TTd11\n"); IE74595_OutByte(dataIE74595&=Den1_ON);}
		else{uart_write_text("TTd10\n"); IE74595_OutByte(dataIE74595|=Den1_OFF);}
		EEPROM_AT24Cxx_write(DEVICE_STATE,dataIE74595); delay_ms(20);
	}
	if(But_Den2 == 0)
	{
		while(But_Den2 == 0);
		if(dataIE74595&Den2_OFF){uart_write_text("TTd21\n"); IE74595_OutByte(dataIE74595&=Den2_ON);}
		else{uart_write_text("TTd20\n"); IE74595_OutByte(dataIE74595|=Den2_OFF);}
		EEPROM_AT24Cxx_write(DEVICE_STATE,dataIE74595); delay_ms(20);
	}
	if(But_Den3 == 0)
	{
		while(But_Den3 == 0);
		if(dataIE74595&Den3_OFF){uart_write_text("TTd31\n"); IE74595_OutByte(dataIE74595&=Den3_ON);}
		else{uart_write_text("TTd30\n"); IE74595_OutByte(dataIE74595|=Den3_OFF);}
		EEPROM_AT24Cxx_write(DEVICE_STATE,dataIE74595); delay_ms(20);
	}
	if(But_Quat1 == 0)
	{
		while(But_Quat1 == 0);
		if(dataIE74595&Quat1_OFF){uart_write_text("TTq11\n"); IE74595_OutByte(dataIE74595&=Quat1_ON);}
		else{uart_write_text("TTq10\n"); IE74595_OutByte(dataIE74595|=Quat1_OFF);}
		EEPROM_AT24Cxx_write(DEVICE_STATE,dataIE74595); delay_ms(20);
	}
	if(But_Quat2 == 0)
	{
		while(But_Quat2 == 0);
		if(dataIE74595&Quat2_OFF){uart_write_text("TTq21\n"); IE74595_OutByte(dataIE74595&=Quat2_ON);}
		else{uart_write_text("TTq20\n"); IE74595_OutByte(dataIE74595|=Quat2_OFF);}
		EEPROM_AT24Cxx_write(DEVICE_STATE,dataIE74595); delay_ms(20);
	}
//	if(But_Quat3 == 0)
//	{
//		while(But_Quat3 == 0);
//		if(dataIE74595&Quat3_OFF){uart_write_text("TTq31\n"); IE74595_OutByte(dataIE74595&=Quat3_ON);}
//		else{uart_write_text("TTq30\n"); IE74595_OutByte(dataIE74595|=Quat3_OFF);}
//		EEPROM_AT24Cxx_write(DEVICE_STATE,dataIE74595); delay_ms(20);
//	}
}

//-------------------------------------------------------------------------
void show_main()
{
	I2C_LCD_WriteCmd(0x0f,0x4e);
	I2C_LCD_Clear(0x4e);
	I2C_LCD_Gotoxy(1,1,0x4e);
	I2C_LCD_PutS("ENTER PASSWORD...",0x4e);
	I2C_LCD_Gotoxy(1,2,0x4e);
	I2C_LCD_PutS("Pin Pass:",0x4e);
}
//-------------------------------------------------------------------------
char Keypad()
{
	while(1)
	{
		cot1 = 1; cot2 = 1; cot3 = 0;
		if(hang4 == 0){while(hang4 == 0);I2C_LCD_PutC('*',0x4e);return '1';}
		if(hang3 == 0){while(hang3 == 0);I2C_LCD_PutC('*',0x4e);return '4';}
		if(hang2 == 0){while(hang2 == 0);I2C_LCD_PutC('*',0x4e);return '7';}
		if(hang1 == 0){while(hang1 == 0);Change_Pin();break;} //ky tu *

		cot1 = 1; cot2 = 0; cot3 = 1;
		if(hang4 == 0){while(hang4 == 0);I2C_LCD_PutC('*',0x4e);return '2';}
		if(hang3 == 0){while(hang3 == 0);I2C_LCD_PutC('*',0x4e);return '5';}
		if(hang2 == 0){while(hang2 == 0);I2C_LCD_PutC('*',0x4e);return '8';}
		if(hang1 == 0){while(hang1 == 0);I2C_LCD_PutC('*',0x4e);return '0';} 

		cot1 = 0; cot2 = 1; cot3 = 1;
		if(hang4 == 0){while(hang4 == 0);I2C_LCD_PutC('*',0x4e);return '3';}
		if(hang3 == 0){while(hang3 == 0);I2C_LCD_PutC('*',0x4e);return '6';}
		if(hang2 == 0){while(hang2 == 0);I2C_LCD_PutC('*',0x4e);return '9';}
		if(hang1 == 0){while(hang1 == 0);Menu_RFID();x=1;break;} //ky tu #
		
		if(CPin_flag == 1) continue;
		
		if(Reset == 0){while(Reset == 0);Reset_pin();break;}
		Button_OpenDoor();
		Button_OnOff_Device();
//		Reset_MasterCard();
		Scan_RFID();
		Uart_handle();
		if(Power == 1 && power_off == 1){power_off = 0; uart_write_text("PWOF\n");}
	}
}

char Keypad_changePin()
{
	while(1)
	{
		cot1 = 1; cot2 = 1; cot3 = 0;
		if(hang4==0){while(hang4==0);I2C_LCD_PutC('1',0x4e);return '1';}
		if(hang3==0){while(hang3==0);I2C_LCD_PutC('4',0x4e);return '4';}
		if(hang2==0){while(hang2==0);I2C_LCD_PutC('7',0x4e);return '7';}

		cot1 = 1; cot2 = 0; cot3 = 1;
		if(hang4==0){while(hang4==0);I2C_LCD_PutC('2',0x4e);return '2';}
		if(hang3==0){while(hang3==0);I2C_LCD_PutC('5',0x4e);return '5';}
		if(hang2==0){while(hang2==0);I2C_LCD_PutC('8',0x4e);return '8';}
		if(hang1==0){while(hang1==0);I2C_LCD_PutC('0',0x4e);return '0';} 

		cot1 = 0; cot2 = 1; cot3 = 1;
		if(hang4==0){while(hang4==0);I2C_LCD_PutC('3',0x4e);return '3';}
		if(hang3==0){while(hang3==0);I2C_LCD_PutC('6',0x4e);return '6';}
		if(hang2==0){while(hang2==0);I2C_LCD_PutC('9',0x4e);return '9';}
	}
}

void Change_Pin()
{
		unsigned char i = 0;
		CPin_flag = 1;
		I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e);
		I2C_LCD_PutS("CHANGE PASSWORD..",0x4e);
		delay_ms(10);
		I2C_LCD_Gotoxy(1,2,0x4e);
		I2C_LCD_PutS("Old Pass:",0x4e);
		while(pin_default[i]!='\0')
		{
			pin_scan[i] = Keypad();
			delay_ms(1);
			i++;
		}
		delay_ms(500);
		CheckOldPin_and_ChangePin();
		show_main();
		CPin_flag = 0;
		x=1;
}

void Check_Pin()
{
	Read_eeprom(&pin_eeprom,200);
	if(pin_scan[0]==pin_eeprom[0]&&pin_scan[1]==pin_eeprom[1]
		&&pin_scan[2]==pin_eeprom[2]&&pin_scan[3]==pin_eeprom[3])
	{
		delay_ms(10);
		I2C_LCD_WriteCmd(0x0c,0x4e); I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("PIN CORRECT...",0x4e);
		buzzer = 0; delay_ms(200); buzzer = 1;
		I2C_LCD_Gotoxy(1,2,0x4e); I2C_LCD_PutS("DOOR OPENED...",0x4e);
		open_and_closed();
	}
	else
	{
		wrong_time++;
		I2C_LCD_WriteCmd(0x0c,0x4e); I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("PIN INCORRECT...",0x4e);
		I2C_LCD_Gotoxy(6,2,0x4e); I2C_LCD_PutNumber(wrong_time,0x4e); I2C_LCD_PutS(" time",0x4e);
		buzzer_ring();
		delay_ms(1000);
		y=1;
	}
	show_main();
}

void CheckOldPin_and_ChangePin()
{
	char i=0;
	Read_eeprom(&pin_eeprom,200);
	if(pin_scan[0]==pin_eeprom[0]&&pin_scan[1]==pin_eeprom[1]
		&&pin_scan[2]==pin_eeprom[2]&&pin_scan[3]==pin_eeprom[3])
	{
		buzzer = 0; delay_ms(200); buzzer = 1;
		I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e);
		I2C_LCD_PutS("CHANGE PASSWORD..",0x4e);
		delay_ms(10);
		I2C_LCD_Gotoxy(1,2,0x4e);
		I2C_LCD_PutS("New Pass:",0x4e);
		while(pin_default[i]!='\0')
		{
			pin_eeprom[i] = Keypad_changePin();
			delay_ms(1);
			i++;
		}
		Write_eeprom(&pin_eeprom,200);
		delay_ms(200);
		buzzer = 0; delay_ms(200); buzzer = 1;
		I2C_LCD_WriteCmd(0x0c,0x4e);
		I2C_LCD_Gotoxy(1,2,0x4e); I2C_LCD_PutS("SUCCESSFUL...",0x4e);
		delay_ms(1000);
	}
	else
	{
		wrong_time++;
		I2C_LCD_WriteCmd(0x0c,0x4e); I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("PIN INCORRECT...",0x4e);
		I2C_LCD_Gotoxy(6,2,0x4e); I2C_LCD_PutNumber(wrong_time,0x4e); I2C_LCD_PutS(" time",0x4e);
		buzzer_ring();
		delay_ms(1000);
		y=1;
	}
}

void Button_OpenDoor()
{
	char i;
	if(Button_Open == 0)
	{
		while(Button_Open == 0);
		uart_write_text("DOP\n"); 
		I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e);
		I2C_LCD_PutS("DOOR OPENED...",0x4e);
		buzzer = 0; delay_ms(200); buzzer = 1;
		servo_open();
		for(i = 5;i >= 0;i--)
		{	
			I2C_LCD_Gotoxy(9,2,0x4e);
			I2C_LCD_PutNumber(i,0x4e);
			I2C_LCD_PutC('s',0x4e);
			delay_ms(1000);
		}
		servo_close();
		show_main();
	}
}

void buzzer_ring()
{
	char i;
	if(wrong_time < 3)
	{
		buzzer=0;delay_ms(200);buzzer=1;delay_ms(100);
		buzzer=0;delay_ms(200);buzzer=1;
	}
	else if(wrong_time >= 3)
	{
		wrong_time=0;
		uart_write_text("WR3\n");
		I2C_LCD_WriteCmd(0x0c,0x4e);
		I2C_LCD_Gotoxy(1,2,0x4e); I2C_LCD_PutS("!!! WARNING !!! ",0x4e);
		for(i=0;i<20;i++){buzzer=!buzzer;delay_ms(200);}
	}
}

void open_and_closed()
{
	char i;
	servo_open();
	uart_write_text("DOP\n");
	for(i = 5;i >= 0;i--)
	{	
		I2C_LCD_Gotoxy(15,2,0x4e);
		I2C_LCD_PutNumber(i,0x4e);
		I2C_LCD_PutC('s',0x4e);
		delay_ms(1000);
	}
	servo_close();
}

void ScanKeyRFID_and_Check()
{
		unsigned char i=0;
		x=0; y=0;
		while(pin_default[i]!='\0')
		{
			pin_scan[i] = Keypad();
			if(x==1) return;
			if(y==1) return;
			i++;
		}
		delay_ms(100);
		Check_Pin();
}

void Reset_pin()
{
	I2C_LCD_WriteCmd(0x0c,0x4e); I2C_LCD_Clear(0x4e);
	I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS(" Reset Password ",0x4e);
	Write_eeprom(&pin_default,200);
	delay_ms(500);
	I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("Reset Successful",0x4e);
	delay_ms(2000);
	show_main();
	x=1;
}

void servo_open()
{
	door = 1; T0_delay_us(2211); door = 0; T0_delay_us(16220);
	door = 1; T0_delay_us(2211); door = 0; T0_delay_us(16220);
}
void servo_close()
{
	door = 1; T0_delay_us(1382); door = 0; T0_delay_us(17050);
	door = 1; T0_delay_us(1382); door = 0; T0_delay_us(17050);
}
//-------------------------------------------------------------------------
void Scan_RFID()
{
	char TagType;
	if(MFRC522_isCard(&TagType))
	{
		if(MFRC522_ReadCardSerial(&UID_Scan))
		{
			I2C_LCD_WriteCmd(0x0c,0x4e); I2C_LCD_Clear(0x4e);
			I2C_LCD_Gotoxy(2,1,0x4e); I2C_LCD_PutS("QUET THE RFID!",0x4e);
			Compare_EEPROM();
			if(CardYes == 1 && CardNo == 0)
			{
				buzzer = 0; delay_ms(200); buzzer = 1;
				delay_ms(1000);
				I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("QUET THANH CONG!",0x4e);
				I2C_LCD_Gotoxy(1,2,0x4e); I2C_LCD_PutS("DOOR OPENED...",0x4e);
				open_and_closed();
			}
			else if(CardYes == 0 && CardNo > 0)
			{
				wrong_time++;
				I2C_LCD_WriteCmd(0x0c,0x4e);
				I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("!!! SAI THE !!! ",0x4e);
				I2C_LCD_Gotoxy(6,2,0x4e); I2C_LCD_PutNumber(wrong_time,0x4e); I2C_LCD_PutS(" time",0x4e);
				buzzer_ring();
				delay_ms(1000);
			}
			show_main();
		}
	}
	MFRC522_Halt();
}

char Check_Card(char UID_Scan[],char Data_Card[])
{
	char i;
	for(i=0;i<4;i++)
	{
		if(UID_Scan[i] == Data_Card[i]) check_id = 1;
		else {check_id = 0; break;}
	}
	return check_id;
}

void Check_Master()
{
	char TagType;
	if(MFRC522_isCard(&TagType))
	{
		if(MFRC522_ReadCardSerial(&UID_Scan))
		{
			if(Check_Card(UID_Scan,Master_id)) check_masterCard = 1;
			else check_masterCard = 0;
		}
	}
	MFRC522_Halt();
}

void Menu_RFID()
{
		I2C_LCD_WriteCmd(0x0c,0x4e);I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("Scan MasterCard!",0x4e);
		while(1)
		{
			Check_Master();
			if(check_masterCard == 1)
			{
				I2C_LCD_Gotoxy(5,2,0x4e); I2C_LCD_PutS("THE DUNG",0x4e); delay_ms(2000);
				check_masterCard = 2; break;
			}
			else if(check_masterCard == 0)
			{
				I2C_LCD_Gotoxy(5,2,0x4e); I2C_LCD_PutS("THE SAI",0x4e); delay_ms(2000);
				check_masterCard = 2; show_main(); return;
			}
		}
		open_menu = 1; Menu_RFID_All(); hang1 = 0; cot1 = 1; cot2 = 1;
		while(open_menu == 1)
		{
			if(cot2 == 0)
			{
				while(cot2 == 0);select_menu++;
				if(select_menu > 2) select_menu = 0;
				Menu_RFID_All();
			}
			if(cot1 == 0){while(cot1 == 0); enter_select = 1;}
			Menu_RFID_Select();
		}
		show_main();
	  open_menu = 0; hang1 = 1;
}

void Menu_RFID_All()
{
	if(open_menu == 1 && select_menu == 0)
	{
		I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS(" THEM & XOA THE ",0x4e);
		I2C_LCD_Gotoxy(1,2,0x4e); I2C_LCD_PutS("> THEM THE MOI  ",0x4e);
	}
	if(open_menu == 1 && select_menu == 1)
	{
		I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("> XOA 1 THE     ",0x4e);
		I2C_LCD_Gotoxy(1,2,0x4e); I2C_LCD_PutS("  XOA ALL THE   ",0x4e);
	}
									
	if(open_menu == 1 && select_menu == 2)
	{
		I2C_LCD_Gotoxy(1,1,0x4e); I2C_LCD_PutS("  XOA 1 THE     ",0x4e);
		I2C_LCD_Gotoxy(1,2,0x4e); I2C_LCD_PutS("> THOAT         ",0x4e);
	}
}

void Menu_RFID_Select()
{
	if(enter_select == 1 && select_menu == 0)
	{
		I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(3,1,0x4e); I2C_LCD_PutS("THEM THE MOI",0x4e);
		I2C_LCD_Gotoxy(3,2,0x4e); I2C_LCD_PutS("MOI NHAP THE",0x4e);
		addNew_Card(); select_menu = 0;
	}
	if(enter_select == 1 && select_menu == 1)
	{
		I2C_LCD_Clear(0x4e);
		I2C_LCD_Gotoxy(4,1,0x4e); I2C_LCD_PutS("XOA 1 THE",0x4e);
		I2C_LCD_Gotoxy(3,2,0x4e); I2C_LCD_PutS("MOI NHAP THE",0x4e);
		Remove_Card(); select_menu = 0;
	}
	if(enter_select == 1 && select_menu == 2)
	{
		open_menu = 0; enter_select = 0; select_menu = 0;
	}
}

//void Reset_MasterCard()
//{
//	unsigned int i;
//	if(P0_4 == 0)
//	{
//		while(P3_4 == 0);
//		I2C_LCD_Clear(0x4e);
//		I2C_LCD_Gotoxy(1,1,0x4e);
//		I2C_LCD_PutS("Reset MasterCard",0x4e);
//		Write_eeprom(&Master_id,1);
//		delay_ms(20);
//		EEPROM_AT24Cxx_write(0,5);
//		delay_ms(500);
//		I2C_LCD_Gotoxy(1,2,0x4e);
//		I2C_LCD_PutS("Reset Successful",0x4e);
//		delay_ms(2000);
//		show_main();
//	}
//	if(P0_5 == 0)
//	{
//		while(P3_4 == 0);
//		I2C_LCD_Clear(0x4e);
//		I2C_LCD_Gotoxy(4,1,0x4e);
//		I2C_LCD_PutS("XOA EEPROM",0x4e);
//		for(i=0;i<EEPROM_AT24Cxx_length();i++)
//		{
//			EEPROM_AT24Cxx_write(i,0);
//			delay_ms(11);
//		}
//		I2C_LCD_Clear(0x4e);
//		I2C_LCD_Gotoxy(2,1,0x4e);
//		I2C_LCD_PutS("XOA THANH CONG",0x4e);
//		delay_ms(2000);
//		show_main();
//	}
//}

void FindEEPROM_Zero()
{
  for(AddrZero = 5; AddrZero < 200; AddrZero++)
  {
		delay_ms(15); 
    if(EEPROM_AT24Cxx_read(AddrZero) == 0) return;     
  }
}

void Compare_EEPROM()
{
	unsigned char CardNumbers, CardAddr = 1;
	CardNumbers = EEPROM_AT24Cxx_read(0);
	delay_ms(15);
  while(CardAddr < CardNumbers)
  {
		Read_eeprom(&UID_EEPROM,CardAddr);
		delay_ms(15);
    if(Check_Card(UID_Scan,UID_EEPROM)){CardYes = 1; CardNo = 0; break;}
		else{CardYes = 0; CardNo += 1;}
		CardAddr += 4;
		CardRemoveAddr = CardAddr;
  }  
}

void Print_AddCard()
{
	I2C_LCD_Gotoxy(1,2,0x4e);
	I2C_LCD_PutS("Save Successfull",0x4e);
	delay_ms(2000);
}

void Scan_NewCard()
{
	char TagType;
	if(MFRC522_isCard(&TagType))
	{
		if(MFRC522_ReadCardSerial(&UID_Scan))
		{
			check_newcard = 1;
		}
	}
	MFRC522_Halt();
}

void addNew_Card()
{
	unsigned int CardNumbers;
	CardNumbers = EEPROM_AT24Cxx_read(0);
	delay_ms(15);
	while(check_newcard == 0)
	{
		Scan_NewCard();
	}
	check_newcard = 0;
	buzzer = 0; delay_ms(200); buzzer = 1;
	while(enter_select == 1)
	{
		if(CardNumbers == 5)
		{
			Write_eeprom(&UID_Scan,5); delay_ms(15);
			EEPROM_AT24Cxx_write(0,9); delay_ms(15);
			Print_AddCard();
			CardYes = 0; CardNo = 0; open_menu = 0; enter_select = 0; 
		}
		else if(CardNumbers > 5)
		{
			Compare_EEPROM();
			if(CardYes == 1 && CardNo == 0) //the da co
			{
				buzzer = 0; delay_ms(200); buzzer = 1;
				I2C_LCD_Gotoxy(1,2,0x4e);
				I2C_LCD_PutS("    The da co   ",0x4e);
				delay_ms(2000);
				CardYes = 0; CardNo = 0; open_menu = 0; enter_select = 0;
			}
			else if(CardYes == 0 && CardNo > 0) //the chua co
			{
				FindEEPROM_Zero();
				if(AddrZero == CardNumbers)
				{
					Write_eeprom(&UID_Scan,AddrZero); delay_ms(15);
					CardNumbers += 4;
					EEPROM_AT24Cxx_write(0,CardNumbers); delay_ms(15);
					Print_AddCard();
				}
				else if(AddrZero < CardNumbers)
				{
					Write_eeprom(&UID_Scan,AddrZero); delay_ms(15);
					Print_AddCard();
				}
				CardYes = 0; CardNo = 0; open_menu = 0; enter_select = 0;
			}
		}
	}
	FindEEPROM_Zero();
}

void Remove_Card()
{
	unsigned int CardNumbers;
	CardNumbers = EEPROM_AT24Cxx_read(0);
	delay_ms(15);
	while(check_newcard == 0)
	{
		Scan_NewCard();
	}
	check_newcard = 0;
	buzzer = 0; delay_ms(200); buzzer = 1;
	while(enter_select == 1)
	{
		if(CardNumbers == 5)
		{
			I2C_LCD_Gotoxy(1,2,0x4e);
			I2C_LCD_PutS("   CHUA CO THE  ",0x4e);
			delay_ms(2000);
			CardYes = 0; CardNo = 0; open_menu = 0; enter_select = 0;
		}
		else if(CardNumbers > 5)
		{
			Compare_EEPROM();
			if(CardYes == 1 && CardNo == 0)
			{
				if((CardRemoveAddr + 4) == CardNumbers)
				{
					FindEEPROM_Zero();
					if(AddrZero < CardRemoveAddr) EEPROM_AT24Cxx_write(0,(CardRemoveAddr-4));
					else if((AddrZero-4) == CardRemoveAddr) EEPROM_AT24Cxx_write(0,CardRemoveAddr);
				}
				delay_ms(20);
//				Write_eeprom(&RemoveCardValue,CardRemoveAddr);
				Write_eeprom("\0\0\0\0",CardRemoveAddr);
				I2C_LCD_Gotoxy(1,2,0x4e);
				I2C_LCD_PutS("   DA XOA THE   ",0x4e);
				delay_ms(2000);
				CardYes = 0; CardNo = 0; open_menu = 0; enter_select = 0;
			}
			else if(CardYes == 0 && CardNo > 0)
			{
				I2C_LCD_Gotoxy(1,2,0x4e);
				I2C_LCD_PutS("    SAI THE     ",0x4e);
				delay_ms(2000);
				CardYes = 0; CardNo = 0; open_menu = 0; enter_select = 0;
			}
		}
	}
	FindEEPROM_Zero();
}

//---------------------------------------------------------
void Write_eeprom(unsigned char *str,unsigned int address)
{
	char i;
	for(i=0;i<4;i++)
	{
		EEPROM_AT24Cxx_write(address,*str);
		delay_ms(11);
		address++;
		str++;
	}
}
void Read_eeprom(unsigned char *str,unsigned int address)
{
	char i;
	for(i=0;i<4;i++)
	{
		*str=EEPROM_AT24Cxx_read(address);
		delay_ms(11);
		address++;
		str++;
	}
}
//--------------------------------------------------------------------
void IE74595_OutByte(unsigned char b)
{
	int i;
	for(i=0;i<8;i++)
	{
		IE74595_DS = (b&0x80)==0x80 ? 1:0;
		delay_ms(1);
		IE74595_SHCP = 1; _nop_();_nop_();_nop_(); IE74595_SHCP = 0;
		b <<= 1;
	}
	IE74595_STCP = 1; _nop_();_nop_();_nop_(); IE74595_STCP = 0;
}
//--------------------------------------------------------------------