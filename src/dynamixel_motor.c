#include <dynamixel_motor.h>
#include "dynamixel_protocol.h"
#include <unistd.h>

#include "dynamixel_constants.h"
#include "dynamixel_translation.h"

uint16_t degrees_to_motor_value(float degrees) {
    if (degrees < 0.0f) degrees = 0.0f;
    if (degrees > 300.0f) degrees = 300.0f;
    const int32_t result = (int32_t)((degrees * 1023.0f) / 300.0f);
    if (result < 0) return 0;
    if (result > 0x3FF) return 0x3FF;
    return (uint16_t)result;
}

float motor_value_to_degrees(uint16_t value) {
    return value * 300.0f / 1023.0f;
}

bool dxl_ping(int connection, uint8_t id) {
    response resp = send_instruction(connection, id, INST_PING, NULL, 0);
    return resp.valid && (resp.error == 0);
}

bool dxl_set_goal_position(int connection, uint8_t id, uint16_t position) {
    return write_register(connection, id, REG_GOAL_POSITION, position, REG_GOAL_POSITION_LEN);
}

bool dxl_set_goal_position_degrees(int connection, uint8_t id, float degrees) {
    return write_register(
        connection,
        id,
        REG_GOAL_POSITION,
        degrees_to_motor_value(degrees),
        REG_GOAL_POSITION_LEN);
}

bool dxl_set_goal_position_multi(int connection, const uint8_t *ids, const uint16_t *positions, int number_of_motors) {
    return sync_write_two_bytes(connection, ids, REG_GOAL_POSITION, number_of_motors, positions);
}

bool dxl_set_moving_speed(int connection, const uint8_t id, uint16_t speed) {
    return write_register(connection, id, REG_MOVING_SPEED, speed, REG_GOAL_POSITION_LEN);
}

bool dxl_set_moving_speed_multi(int connection, const uint8_t *ids, const uint16_t *speeds, int number_of_motors) {
    return sync_write_two_bytes(connection, ids, REG_GOAL_POSITION, number_of_motors, speeds);
}

bool dxl_is_moving(int connection, uint8_t id) {
    unsigned int moving_status = 0;
    if (read_register(connection, id, REG_MOVING, &moving_status, REG_MOVING_LEN)) {
        return (moving_status == 0x01);
    }
    return false;  // Assume stopped if read fails
}

void dxl_wait_until_stopped(int connection, uint8_t id) {
    while (dxl_is_moving(connection, id)) {
        usleep(100000);  // 100ms polling interval
    }
}

void dxl_wait_until_all_stopped(int connection, uint8_t *ids, int number_of_motors) {
    for (int i = 0; i < number_of_motors; i++) {
        dxl_wait_until_stopped(connection, ids[i]);
    }
}


