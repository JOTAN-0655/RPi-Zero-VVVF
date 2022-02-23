#ifndef DELAY_H

#define DELAY_H

volatile unsigned long long get_systime(void);
void delay_ms(unsigned int delay);
void delay_us(unsigned int delay);

#endif