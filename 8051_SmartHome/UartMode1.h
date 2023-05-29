
//thu vien giao tiep UART Kim Nhut Hoang 10DHDT1

#ifndef _UARTMODE1_H_
#define _UARTMODE1_H_
/*********************************Chuong trinh mau****************************************
void main()
{
	uart_init();
	while(uart_data_ready()==1);
	/
	tiep theo la cac lenh doc hay ghi tuy nguoi su dung
	/
	while(1)
	{
		
	}
}
*****************************************************************************************/
//khoi tao UART mode 1
void uart_init();

//khoi tao ngat port noi tiep nhan du lieu
void uart_isrInit();

//gui 1 ky tu ra UART
void uart_write(char c);

//gui chuoi ky tu ra UART
void uart_write_text(char *str);

//chuyen so hex sang so thap phan de gui
void uart_write_number(char num);

//kiem tra nhan duoc data hay chua
char uart_data_ready();

//doc data nhan duoc tu UART
unsigned char uart_read();

#endif