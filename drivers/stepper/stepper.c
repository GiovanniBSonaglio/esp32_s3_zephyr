#include <zephyr/drivers/stepper/stepper.h>
#include <zephyr/drivers/stepper/stepper_ctrl.h>

#include <zephyr/logging/log.h>

#include "stepper.h"

LOG_MODULE_REGISTER(stepper_drv, LOG_LEVEL_WRN);

static const struct device *stepper_driver = DEVICE_DT_GET(DT_ALIAS(stepper_driver));
static const struct device *stepper_ctrl = DEVICE_DT_GET(DT_ALIAS(stepper_ctrl));

#define STEPPER_DRIVER_MICRO_STEP_RES DT_PROP_OR(DT_ALIAS(stepper_driver), micro_step_res, 1)
#define STEPPER_PI 3.14159265358979323846f

/* Carries the fractional microstep lost to rounding into the next call,
 * to avoid drifting issues. */
static float microstep_remainder;

int init_stepper(){
    int ret = 0;

	if (!device_is_ready(stepper_ctrl)) {
		LOG_ERR("Device %s is not ready", stepper_ctrl->name);
		return -ENODEV;
	}

	if (!device_is_ready(stepper_driver)) {
		LOG_ERR("Device %s is not ready", stepper_driver->name);
		return -ENODEV;
	}

    LOG_DBG("stepper is %p, name is %s", stepper_ctrl, stepper_ctrl->name);

    ret = stepper_enable(stepper_driver);
    if(ret < 0) {
        LOG_ERR("Failed to enable stepper driver (errno=%d)", ret);
        return ret;
    }

    return ret;
}

static int stepper_move_by_microsteps_frac(float microsteps)
{
    float total = microsteps + microstep_remainder;
    int32_t whole = (int32_t)total;

    microstep_remainder = total - (float)whole;

    return stepper_move_by_microsteps(whole);
}

int stepper_move_by_microsteps(int32_t microsteps)
{
    int ret = stepper_ctrl_move_by(stepper_ctrl, microsteps);
    if (ret < 0) {
        LOG_ERR("Error moving stepper (errno=%d)", ret);
    }
    return ret;
}

int stepper_move_by_deg(float angle_deg)
{
    float microsteps_per_rev = CONFIG_STEPPER_FULL_STEPS_PER_REV * STEPPER_DRIVER_MICRO_STEP_RES;

    return stepper_move_by_microsteps_frac(angle_deg * microsteps_per_rev / 360.0f);
}

int stepper_move_by_rad(float angle_rad)
{
    float microsteps_per_rev = CONFIG_STEPPER_FULL_STEPS_PER_REV * STEPPER_DRIVER_MICRO_STEP_RES;

    return stepper_move_by_microsteps_frac(angle_rad * microsteps_per_rev / (2.0f * STEPPER_PI));
}