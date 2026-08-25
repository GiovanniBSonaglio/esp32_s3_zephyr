#include <string.h>
#include <errno.h>

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
	const uint8_t msg[] = {'t', 'e', 's', 't'};
    uint8_t buf[4];

    int written = uart_write(msg, sizeof(msg), K_FOREVER);
    zassert_equal(written, (int)sizeof(msg), "uart_write returned %d", written);

    int got = uart_emul_get_tx_data(emul_dev, buf, sizeof(buf));
    zassert_equal(got, (int)sizeof(msg), "captured %d bytes", got);
    zassert_mem_equal(buf, msg, sizeof(msg));
}

/**
 * @brief Test Uart Rx
 *
 * This test verifies if the uart Rx is working.
 *
 */
ZTEST(uart_unit_test, test_uart_rx)
{
	const uint8_t msg[] = {'t', 'e', 's', 't'};
	uint8_t ch;

	uart_emul_put_rx_data(emul_dev, msg, sizeof(msg));
	
	for(int i = 0; i < (int)sizeof(msg); i++) {
		uart_get_byte(&ch, K_FOREVER);
		zassert_equal(ch, msg[i], "Character in pos %d does not match", i);
	}
}
