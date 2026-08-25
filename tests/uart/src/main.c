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

/* Payload larger than the driver's TX ring buffer -> exercises the chunk-and-wait loop. */
ZTEST(uart_unit_test, test_uart_tx_large_payload)
{
	uint8_t out[512];
	uint8_t in[512];

	for (size_t i = 0; i < sizeof(out); i++) {
		out[i] = (uint8_t)i;
	}

	int written = uart_write(out, sizeof(out), K_FOREVER);
	zassert_equal(written, (int)sizeof(out), "did not write the whole payload (%d)", written);

	int got = uart_emul_get_tx_data(emul_dev, in, sizeof(in));
	zassert_equal(got, (int)sizeof(out), "emul did not capture the whole payload (%d)", got);
	zassert_mem_equal(in, out, sizeof(out), "large TX payload mismatch");
}

/* Zero-length write is a no-op. */
ZTEST(uart_unit_test, test_uart_tx_zero_length)
{
	uint8_t src[4] = {1, 2, 3, 4};
	uint8_t sink[4];

	int written = uart_write(src, 0, K_FOREVER);
	zassert_equal(written, 0, "zero-length write should report 0 bytes (%d)", written);

	int got = uart_emul_get_tx_data(emul_dev, sink, sizeof(sink));
	zassert_equal(got, 0, "nothing should have been transmitted (%d)", got);
}

/* Write of exactly the ring-buffer size. */
ZTEST(uart_unit_test, test_uart_tx_exact_buffer_size)
{
	uint8_t out[CONFIG_UART_TX_BUFFER_SZ];
	uint8_t in[CONFIG_UART_TX_BUFFER_SZ];

	memset(out, 'X', sizeof(out));

	int written = uart_write(out, sizeof(out), K_FOREVER);
	zassert_equal(written, (int)sizeof(out), "did not write the full buffer (%d)", written);

	int got = uart_emul_get_tx_data(emul_dev, in, sizeof(in));
	zassert_equal(got, (int)sizeof(out), "emul did not capture the full buffer (%d)", got);
	zassert_mem_equal(in, out, sizeof(out), "exact-size payload mismatch");
}

/* Non-blocking read on an empty buffer returns -EAGAIN. */
ZTEST(uart_unit_test, test_uart_rx_nowait_empty)
{
	unsigned char c;

	int ret = uart_get_byte(&c, K_NO_WAIT);
	zassert_equal(ret, -EAGAIN, "empty non-blocking read should return -EAGAIN, got %d", ret);
}

/* A large ordered RX payload round-trips in order. */
ZTEST(uart_unit_test, test_uart_rx_large_ordered)
{
	uint8_t payload[200];

	for (size_t i = 0; i < sizeof(payload); i++) {
		payload[i] = (uint8_t)i;
	}

	int put = uart_emul_put_rx_data(emul_dev, payload, sizeof(payload));
	zassert_equal(put, (int)sizeof(payload), "emul did not accept the RX payload (%d)", put);

	for (size_t i = 0; i < sizeof(payload); i++) {
		unsigned char c;
		int ret = uart_get_byte(&c, K_MSEC(100));

		zassert_equal(ret, 0, "byte %u not received (%d)", (unsigned)i, ret);
		zassert_equal(c, payload[i], "byte %u out of order (got %u)", (unsigned)i, c);
	}
}

/* Two threads writing concurrently must not interleave (per-message atomicity). */
#define TX_THREAD_STACK 1024
#define TX_PATTERN_LEN  100

K_THREAD_STACK_DEFINE(tx_stack_a, TX_THREAD_STACK);
K_THREAD_STACK_DEFINE(tx_stack_b, TX_THREAD_STACK);
static struct k_thread tx_thread_a;
static struct k_thread tx_thread_b;

static void tx_writer(void *pattern_char, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	uint8_t buf[TX_PATTERN_LEN];

	memset(buf, (int)(uintptr_t)pattern_char, sizeof(buf));
	uart_write(buf, sizeof(buf), K_FOREVER);
}

ZTEST(uart_unit_test, test_uart_tx_two_threads)
{
	uint8_t captured[2 * TX_PATTERN_LEN];

	k_thread_create(&tx_thread_a, tx_stack_a, TX_THREAD_STACK, tx_writer,
			(void *)(uintptr_t)'A', NULL, NULL, 5, 0, K_NO_WAIT);
	k_thread_create(&tx_thread_b, tx_stack_b, TX_THREAD_STACK, tx_writer,
			(void *)(uintptr_t)'B', NULL, NULL, 5, 0, K_NO_WAIT);

	k_thread_join(&tx_thread_a, K_FOREVER);
	k_thread_join(&tx_thread_b, K_FOREVER);

	int got = uart_emul_get_tx_data(emul_dev, captured, sizeof(captured));
	zassert_equal(got, (int)sizeof(captured), "both writes should be captured (%d)", got);

	/* Each thread's block must be contiguous, never interleaved. */
	uint8_t first = captured[0];
	uint8_t second = (first == 'A') ? 'B' : 'A';

	zassert_true(first == 'A' || first == 'B', "unexpected first byte %u", first);
	for (int i = 0; i < TX_PATTERN_LEN; i++) {
		zassert_equal(captured[i], first, "first block interleaved at %d", i);
	}
	for (int i = TX_PATTERN_LEN; i < 2 * TX_PATTERN_LEN; i++) {
		zassert_equal(captured[i], second, "second block interleaved at %d", i);
	}
}
