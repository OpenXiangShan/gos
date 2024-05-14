#ifndef _UART_UARTLITE_H__
#define _UART_UARTLITE_H__

#define UARTLITE_RX_FIFO  0x0
#define UARTLITE_TX_FIFO  0x4
#define UARTLITE_STAT_REG 0x8
#define UARTLITE_CTRL_REG 0xc

#define UARTLITE_RST_FIFO 0x03
#define UARTLITE_TX_FULL  0x08
#define UARTLITE_RX_VALID 0x01

void uart_uartlite_init(unsigned long base, struct sbi_uart_ops *ops);

#endif