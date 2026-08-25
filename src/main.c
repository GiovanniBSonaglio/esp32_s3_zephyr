#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "uart.h"
#include "usbd_configurator.h"

LOG_MODULE_REGISTER(esp32_app_module, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const uart_dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);

static struct usbd_context usbd_ctx;

int main(void)
{
    int ret;

    if(init_uart(uart_dev)) {
        LOG_ERR("Error initializing UART");
        return -1;
    }

    ret = init_cdc_acm(&usbd_ctx);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB CDC ACM device support");
		return ret;
	}

    LOG_INF("ESP32 Successfully booted");

    return 0;
}