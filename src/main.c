#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "uart.h"

LOG_MODULE_REGISTER(esp32_app_module, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

int main(void)
{
    
    if(init_uart(console)) {
        LOG_ERR("Error initializing UART");
        return -1;
    }
    LOG_INF("esp32-drivers: Boot ok");

    return 0;
}