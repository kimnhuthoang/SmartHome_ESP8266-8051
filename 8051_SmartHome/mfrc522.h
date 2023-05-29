

#ifndef _mfrc522_h_
#define _mfrc522_h_

#define MFRC522_CS 	P2_1                 
#define MFRC522_SCK P2_2
#define MFRC522_SI  P2_3                           
#define MFRC522_SO  P2_4              
#define MFRC522_RST P2_5

void MFRC522_Init();
char MFRC522_isCard(char *TagType) ;
unsigned char MFRC522_Rd(unsigned char Address);
void MFRC522_Wr(unsigned char Address, unsigned char value);
void MFRC522_Reset();
void MFRC522_AntennaOn();
void MFRC522_AntennaOff();
char MFRC522_ToCard(char command,char *sendData,char sendLen,char *backData,unsigned *backLen);
char MFRC522_Request(char reqMode,char *TagType);
void MFRC522_CRC(char *dataIn,char length,char *dataOut);
char MFRC522_SelectTag(char *serNum );
void MFRC522_Halt();
char MFRC522_Auth(char authMode,char BlockAddr,char *Sectorkey,char *serNum);
char MFRC522_Write(char blockAddr,char *writeData);
char MFRC522_Read(char blockAddr,char *recvData );
char MFRC522_AntiColl(char *serNum );
unsigned char MFRC522_ReadCardSerial(unsigned char *str );

#endif