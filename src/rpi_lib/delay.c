#define GPFSEL0 0x20200000
#define GPFSEL1 0x20200004
#define GPFSEL2 0x20200008
#define GPFSEL3 0x2020000C
#define GPFSEL4 0x20200010
#define GPFSEL5 0x20200014

#define GPSET0  0x2020001C
#define GPSET1  0x20200020
#define GPCLR0  0x20200028
#define GPCLR1  0x2020002C

#define SYST_CLO 0x20003004
#define SYST_CHI 0x20003008

volatile unsigned long long get_systime(void) {
  unsigned long long t;
  unsigned int chi;
  unsigned int clo;

  chi = *(volatile unsigned int *) SYST_CHI;
  clo = *(volatile unsigned int *) SYST_CLO;
  if (chi != *(volatile unsigned int *) SYST_CHI) {
    chi = *(volatile unsigned int *) SYST_CHI;
    clo = *(volatile unsigned int *) SYST_CLO;
  }
  t = (unsigned long long)chi;
  t = t << 32;
  t += (unsigned long long)clo;
  return t;
}

void delay_ms(unsigned int delay){
  unsigned long long alermTime;

  alermTime = get_systime() + delay * 1000;
  while(get_systime() < alermTime);
  
  return;
}

void delay_us(unsigned int delay){
  unsigned long long alermTime;

  alermTime = get_systime() + delay;
  while(get_systime() < alermTime);
  
  return;
}