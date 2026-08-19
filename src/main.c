#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(esp32_app_module, CONFIG_LOG_DEFAULT_LEVEL); 

int main()
{
    LOG_INF("esp32-drivers: Boot ok");

    return 0;
}