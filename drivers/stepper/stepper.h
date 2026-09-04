#ifndef STEPPER_H
#define STEPPER_H

int init_stepper(void);
int stepper_move_by_microsteps(int32_t microsteps);
int stepper_move_by_deg(float angle_deg);
int stepper_move_by_rad(float angle_rad);

#endif // STEPPER_H
