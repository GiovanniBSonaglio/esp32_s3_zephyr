#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include "uart.h"

LOG_MODULE_REGISTER(esp32_uart_driver, LOG_LEVEL_WRN);

K_SEM_DEFINE(uart_tx_sem, 0, 1);
RING_BUF_DECLARE(uart_tx_ringbuf, CONFIG_UART_TX_BUFFER_SZ);
K_MUTEX_DEFINE(uart_tx_mut);

K_SEM_DEFINE(uart_rx_sem, 0, 1);
RING_BUF_DECLARE(uart_rx_ringbuf, CONFIG_UART_RX_BUFFER_SZ);

const struct device *uart_dev = NULL;

static void uart_cb(const struct device *dev, void *user_data) {
    unsigned char buf;

    uart_irq_update(dev);

    while (uart_irq_is_pending(dev)) {

        if(uart_irq_rx_ready(dev)) {
            while(uart_fifo_read(dev, &buf, 1)) {
                ring_buf_put(&uart_rx_ringbuf, &buf, 1);
            }
            k_sem_give(&uart_rx_sem);
        }
        
        if(uart_irq_tx_ready(dev)) {
            if (ring_buf_get(&uart_tx_ringbuf, &buf, 1) == 1) {
                uart_fifo_fill(dev, &buf, 1);
            } else {
                uart_irq_tx_disable(dev);
                k_sem_give(&uart_tx_sem);
            }
        }

        uart_irq_update(dev);
    }
}

int init_uart(const struct device *dev) {
    int status = 0;
    
    if(!device_is_ready(dev)) {
        LOG_ERR("UART device not ready");
        return -ENODEV;
    }
    uart_dev = dev;
    
    status = uart_irq_callback_user_data_set(dev, uart_cb, NULL);
    if(status) {
        LOG_ERR("Error setting IRQ callback.");
        return status;
    }

    uart_irq_rx_enable(dev);

    return status;
}

int uart_get_byte(unsigned char *buf, k_timeout_t timeout) {
    if (ring_buf_get(&uart_rx_ringbuf, buf, 1)) {
        return 0;
    }
    // When no more bytes retrieved in buffer, try to take the semaphore
    if (k_sem_take(&uart_rx_sem, timeout) != 0) {
        return -EAGAIN;
    }
    return (ring_buf_get(&uart_rx_ringbuf, buf, 1) == 1) ? 0 : -EAGAIN;
}

int uart_write_str(const char *s, k_timeout_t timeout)
{
    int ret = 0;
    size_t len = strlen(s);
    size_t offset = 0;

    k_mutex_lock(&uart_tx_mut, K_FOREVER);
    while (offset < len) {
        offset += ring_buf_put(&uart_tx_ringbuf,
                    (const uint8_t *)&s[offset], len - offset);
        uart_irq_tx_enable(uart_dev);
        if (k_sem_take(&uart_tx_sem, timeout) != 0) {  /* wait for this chunk to drain */
            ret = -EAGAIN;
            break;
        }
    }
    k_mutex_unlock(&uart_tx_mut);
    return ret;
}