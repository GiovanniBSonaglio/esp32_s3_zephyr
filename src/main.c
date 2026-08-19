#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "uart.h"

LOG_MODULE_REGISTER(esp32_app_module, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

int main(void)
{
    LOG_INF("esp32-drivers: Boot ok");

    if(init_uart(console)) {
        LOG_ERR("Error initializing UART");
        return -1;
    }

    uart_write_str("test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,test writing a string,ENDOFTEXT\n", K_FOREVER);

    unsigned char buf;
    while (1) {
        if (uart_get_byte(&buf, K_FOREVER) == 0) {
            LOG_INF("Rx Uart: %c", buf);
        }
    }

    return 0;
}