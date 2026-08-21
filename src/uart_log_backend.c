#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_backend_std.h>
#include <zephyr/logging/log_output.h>

#include "uart_log_backend.h"
#include "uart.h"

static uint8_t log_buf[256];

static volatile bool panic_mode;

static int out_cb_fn(uint8_t *data, size_t length, void *ctx)
{
    if (panic_mode) {
        uart_write_sync(data, length);
        return length;
    }
    return uart_write(data, length, K_FOREVER);
}

LOG_OUTPUT_DEFINE(log_output_inst, out_cb_fn, log_buf, sizeof(log_buf));

static void process_log_cb(const struct log_backend *const backend, union log_msg_generic *msg) {
    log_output_msg_process(&log_output_inst, &msg->log, LOG_OUTPUT_FLAG_LEVEL | LOG_OUTPUT_FLAG_TIMESTAMP);
}

static void panic_log_cb(const struct log_backend *const backend) {
    panic_mode = true;
    log_backend_std_panic(&log_output_inst);
}

const struct log_backend_api log_backend_inst_api = {
    .process = process_log_cb,
    .panic = panic_log_cb,
};

LOG_BACKEND_DEFINE(uart_log_backend, log_backend_inst_api, true);
