#include "rpi_address.h"
#include "uart.h"
#include "gpio.h"
#include <math.h>

void UartInit()
{
	*UART_CR = 0;
	
	pinMode(14, PULLDOWN);
	pinMode(15, PULLDOWN);
	pinMode(14, ALT0);
	pinMode(15, ALT0);

	*UART_IBRD = 1;
	*UART_FBRD = 40;
	
	*UART_LCRH = 0x70;
	
	*UART_CR = 0x301;
}

int UartPutc(int c)
{
	while(*UART_FR & (0x01 << 5)){
		;
	}
	*UART_DR = 0xff & c;
	return 1;
}

int UartPuts(char* s)
{
	int i = 0;
	while(*s != '\n' && *s != '\0'){
		UartPutc(*s);
		i++;
		s++;
	}
	if(*s == '\n'){
		UartPutc('\n');
		i++;
	}
	return i;
}

int UartPuts_len(char* s,unsigned len){
	int i = len;
	while(i--){
		UartPutc(*s);
		s++;
	}
	return i;
}

void UartPut_I(int i,char digits){
	for(int x = 0; x < digits;x++){
		int divide = pow(10,(digits-1-x));
		int val_p1 = (int)((double)i/(double)divide);
		int val = val_p1%10;
		UartPutc(val);
	}
}

void UartPut_D(double val,char digits){
	UartPut_I((int)(val * pow(10,digits-1)),digits);
}