#ifndef DYNAMIXEL_MOTOR_H
#define DYNAMIXEL_MOTOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// --- High-Level Motor Control ---

/**
 * Sets the target goal position for a single Dynamixel motor.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the target motor.
 * @param position The goal position value (0x0-0x03FF).
 * @return true if response contains no errors, false otherwise.
 */
bool dxl_set_goal_position(int connection, uint8_t id, uint16_t position);

/**
 * Sets the target goal positions for multiple motors simultaneously.
 *
 * This function is more efficient than calling dxl_set_goal_position() in a loop
 * as it sends a single instruction packet to the bus.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param ids An array of motor IDs to control.
 * @param positions An array of goal positions corresponding to the IDs.
 * @param number_of_motors The number of motors in the arrays.
 * @return true if response contains no errors, false otherwise.
 */
bool dxl_set_goal_position_multi(int connection, const uint8_t *ids, const uint16_t *positions, int number_of_motors);

/**
 * Sets the target goal position for a single motor using degrees.
 *
 * This function converts the degree input into the corresponding raw motor position value
 * before sending the command. It is recommended to use dxl_set_goal_position if you need
 * to be precise with the positions.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the target motor.
 * @param degrees The desired position in degrees (0.0 to 300.0).
 * @return true if response contains no errors, false otherwise.
 */
bool dxl_set_goal_position_degrees(int connection, uint8_t id, float degrees);

/**
 * Sets the moving speed for a single Dynamixel motor.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the target motor.
 * @param speed The moving speed value (e.g., 0-1023). 0 usually means maximum RPM control possible without speed control.
 * @return true if response contains no errors, false otherwise.
 */
bool dxl_set_moving_speed(int connection, uint8_t id, uint16_t speed);

/**
 * Sets the moving speed for multiple motors simultaneously.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param ids An array of motor IDs to control.
 * @param speeds An array of speed values corresponding to the IDs.
 * @param number_of_motors The number of motors in the arrays.
 * @return true if response contains no errors, false otherwise.
 */
bool dxl_set_moving_speed_multi(int connection, const uint8_t *ids, const uint16_t *speeds, int number_of_motors);

/**
 * Checks if a specific motor is currently moving.
 *
 * This usually reads the 'Moving' status bit from the motor's control table.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the target motor.
 * @return true if the motor is moving, false if it is stopped or communication failed.
 */
bool dxl_is_moving(int connection, uint8_t id);

/**
 * Blocks execution until the specified motor has stopped moving under its own power.
 *
 * This function continuously polls the motor status.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the target motor.
 */
void dxl_wait_until_stopped(int connection, uint8_t id);

/**
 * Blocks execution until all specified motors have stopped moving.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param ids An array of motor IDs to monitor.
 * @param number_of_motors The number of motors in the array.
 */
void dxl_wait_until_all_stopped(int connection, uint8_t *ids, int number_of_motors);

/**
 * Pings a motor to verify its existence and connection status.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the target motor.
 * @return true if the motor responded to the ping, false otherwise.
 */
bool dxl_ping(int connection, uint8_t id);

#endif
