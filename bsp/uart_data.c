#include "uart_data.h"
#include "gos.h"

#if CONFIG_SELECT_MELLITE_FPGA
struct uart_data ns16550_uart_data = {
	.clk = 200000000,
	.baud = 115200,
};
#else
struct uart_data ns16550_uart_data = {
	.clk = 50000000,
	.baud = 115200,
};
#endif

struct uart_data qemu8250_uart_data = {
	.clk = 1843200,
	.baud = 115200,
};
