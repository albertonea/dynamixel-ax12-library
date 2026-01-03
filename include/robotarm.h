#ifndef DYNAMIXEL_MOTOR_H
#define DYNAMIXEL_MOTOR_H

#include <stdbool.h>
#include <stdlib.h>

// --- High-Level Motor Control ---
bool dxl_set_goal_position(int connection, unsigned char id, uint16_t position);

bool dxl_set_moving_speed(int connection, unsigned char id, uint16_t speed);

bool dxl_is_moving(int connection, unsigned char id);

void dxl_wait_until_stopped(int connection, unsigned char id);

#endif
