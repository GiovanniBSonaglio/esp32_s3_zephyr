#ifndef UART_H
#define UART_H

int init_uart(const struct device *dev);
int uart_get_byte(unsigned char *ch, k_timeout_t timeout);
int uart_write(const uint8_t *data, size_t len, k_timeout_t timeout);

#endif // UART_H