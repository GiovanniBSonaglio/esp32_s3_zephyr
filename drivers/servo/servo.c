#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

#include "servo.h"

LOG_MODULE_REGISTER(servo_drv, LOG_LEVEL_WRN);

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);

int set_servo_deg_pos(int pos_deg) {
    int ret = 0;
    uint32_t pulse_width = min_pulse + (max_pulse - min_pulse) * pos_deg / 180;

    if (!pwm_is_ready_dt(&servo)) {
		LOG_ERR("Servo PWM device %s is not ready", servo.dev->name);
        return -ENODEV;
	}

    if(pulse_width < min_pulse || pulse_width > max_pulse) {
        return -EINVAL;
    }
    
    ret = pwm_set_pulse_dt(&servo, pulse_width);
    if (ret < 0) {
        LOG_ERR("Failed to set pulse width (errno=%d)", ret);
        return ret;
    }

    return ret;
}