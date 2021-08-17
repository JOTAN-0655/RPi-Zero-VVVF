#ifndef __UART_H__
#define __UART_H__

extern void UartInit();
extern int UartPutc(int c);
extern int UartPuts(char* s);
extern int UartPuts_len(char* s,unsigned len);
extern void UartPut_I(int i,char digits);
extern void UartPut_D(double val,char digits);
#endif