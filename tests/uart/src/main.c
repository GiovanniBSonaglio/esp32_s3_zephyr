#include <string.h>

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/serial/uart_emul.h>

#include "uart.h"

static const struct device *const emul_dev = DEVICE_DT_GET(DT_NODELABEL(uart_test));

static void uart_before(void *fixture)
{
    ARG_UNUSED(fixture);
    uart_emul_flush_rx_data(emul_dev);
    uart_emul_flush_tx_data(emul_dev);
    zassert_equal(init_uart(emul_dev), 0, "Error initializing UART");
}
ZTEST_SUITE(uart_unit_test, NULL, NULL, uart_before, NULL, NULL);

/**
 * @brief Test Uart Tx
 *
 * This test verifies if the uart tx is working.
 *
 */
ZTEST(uart_unit_test, test_uart_tx)
{
	uint8_t buf[4];
	uint8_t *data = &buf[0];
	int data_len;

	uart_write("test", 4, K_FOREVER);
	data_len = uart_emul_get_tx_data(emul_dev, data, 4);
	zassert_equal(data_len, 4, "Incorrect written data length");
	zassert_mem_equal(data, "test", 4);
}

/**
 * @brief Test Uart Rx
 *
 * This test verifies if the uart Rx is working.
 *
 */
ZTEST(uart_unit_test, test_uart_rx)
{
	char test_str[] = "test";
	uint8_t buf;

	uart_emul_put_rx_data(emul_dev, "test", 4);
	
	for(uint8_t i = 0; i < strlen(test_str); i++) {
		uart_get_byte(&buf, K_FOREVER);
		zassert_equal(buf, test_str[i], "Character in pos %d does not match", i);
	}
}
