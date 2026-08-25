#ifndef USBD_CONFIGURATOR
#define USBD_CONFIGURATOR

#include <zephyr/usb/usbd.h>

int init_cdc_acm(struct usbd_context *usbd_ctx);

#endif // USBD_CONFIGURATOR