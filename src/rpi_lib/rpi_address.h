#ifndef __RPI_ADDRESS_H__
#define __RPI_ADDRESS_H__

#define PERI_ADDR(x) (0x20000000 + (x))

#define GPIO_ADDRESS_BASE 0x00200000

#define GPIFSEL0 ((volatile unsigned int *)PERI_ADDR(GPIO_ADDRESS_BASE + 0x00))
#define GPIFSEL1 ((volatile unsigned int *)PERI_ADDR(GPIO_ADDRESS_BASE + 0x04))
#define GPIFSEL2 ((volatile unsigned int *)PERI_ADDR(GPIO_ADDRESS_BASE + 0x08))
#define GPIFSEL3 ((volatile unsigned int *)PERI_ADDR(GPIO_ADDRESS_BASE + 0x0C))
#define GPIFSEL4 ((volatile unsigned int *)PERI_ADDR(GPIO_ADDRESS_BASE + 0x10))
#define GPIFSEL5 ((volatile unsigned int *)PERI_ADDR(GPIO_ADDRESS_BASE + 0x14))

#define GPSET0	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x1C))
#define GPSET1	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x20))

#define GPCLR0	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x28))
#define GPCLR1	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x2C))

#define GPLEV0	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x34))
#define GPLEV1	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x38))

#define GPREN0  ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x4C))
#define GPREN1  ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x50))

#define GPFEN0  ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x58))
#define GPFEN1  ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x5C))

#define GPHEN0  ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x64))
#define GPHEN1  ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x68))

#define GPLEN0	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x70))
#define GPLEN1	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x74))

#define GPAREN0 ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x7C))
#define GPAREN1 ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x80))

#define GPAFEN0 ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x88))
#define GPAFEN1 ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x8C))



#define GPEDS0 ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x40))
#define GPEDS1 ((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x44))

#define GPPUD	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x94))
#define GPPUDCKL0	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x98))
#define GPPUDCKL1	((volatile unsigned int*)PERI_ADDR(GPIO_ADDRESS_BASE + 0x9C))

#define UART_BASE 0x00201000
#define UART_DR ((volatile unsigned int*)PERI_ADDR(UART_BASE + 0x00))
#define UART_FR ((volatile unsigned int*)PERI_ADDR(UART_BASE + 0x18))
#define UART_IBRD ((volatile unsigned int*)PERI_ADDR(UART_BASE + 0x24))
#define UART_FBRD ((volatile unsigned int*)PERI_ADDR(UART_BASE + 0x28))
#define UART_LCRH ((volatile unsigned int*)PERI_ADDR(UART_BASE + 0x2c))
#define UART_CR ((volatile unsigned int*)PERI_ADDR(UART_BASE + 0x30))

#define SYSTEMTIMER_ADDRESS_BASE 0x00003000

#define SYSTEMTIMER_CLO ((volatile unsigned int*)PERI_ADDR(SYSTEMTIMER_ADDRESS_BASE + 0x04))
#define SYSTEMTIMER_CHI ((volatile unsigned int*)PERI_ADDR(SYSTEMTIMER_ADDRESS_BASE + 0x08))

#define MAILBOX_ADDRESS_BASE 0x0000B880

#define MAILBOX_READ   ((volatile unsigned int*)PERI_ADDR(MAILBOX_ADDRESS_BASE + 0x00))
#define MAILBOX_STATUS ((volatile unsigned int*)PERI_ADDR(MAILBOX_ADDRESS_BASE + 0x18))
#define MAILBOX_WRITE  ((volatile unsigned int*)PERI_ADDR(MAILBOX_ADDRESS_BASE + 0x20))

#define BSC1_ADDRESS_BASE 0x00804000

#define BSC1_C		((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x00))
#define BSC1_S		((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x04))
#define BSC1_DLEN	((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x08))
#define BSC1_A		((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x0C))
#define BSC1_FIFO	((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x10))
#define BSC1_DIV	((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x14))
#define BSC1_DEL	((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x18))
#define BSC1_CLKT	((volatile unsigned int *)PERI_ADDR(BSC1_ADDRESS_BASE + 0x1C))

#endif
