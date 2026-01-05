#ifndef DYNAMIXEL_MOTOR_H
#define DYNAMIXEL_MOTOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// --- High-Level Motor Control ---
bool dxl_set_goal_position(int connection, uint8_t id, uint16_t position);

bool dxl_set_goal_position_multi(int connection, const uint8_t *ids, const uint16_t *positions, int number_of_motors);

bool dxl_set_moving_speed(int connection, uint8_t id, uint16_t speed);

bool dxl_set_moving_speed_multi(int connection, const uint8_t *ids, const uint16_t *speeds, int number_of_motors);

bool dxl_is_moving(int connection, uint8_t id);

void dxl_wait_until_stopped(int connection, uint8_t id);

void dxl_wait_until_all_stopped(int connection, uint8_t *ids, int number_of_motors);

bool dxl_ping(int connection, uint8_t id);

#endif
