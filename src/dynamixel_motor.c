#include <robotarm.h>
#include "dynamixel_protocol.h"
#include <unistd.h>

bool dxl_write_register(
    int connection,
    unsigned char id,
    unsigned char address,
    unsigned int value,
    int register_size
    ) {
    unsigned char params[register_size + 1];
    params[0] = address;

    // create params from register size
    for (int i = 1; i < register_size + 1; i++) {
        params[i] = (unsigned char)(value & 0xFF);
        value >>= 8;
    }

    response resp = send_instruction(connection, id, INST_WRITE, params, 3);
    return resp.valid && (resp.error == 0);
}

bool dxl_sync_write(
    const int connection,
    const unsigned char *ids,
    const unsigned char address,
    const int number_of_motors,
    const uint16_t *values,
    const int register_size
    ) {
    // (L + 1) * N + 2 (L: Data length for each Dynamixel actuator, N: The number of Dynamixel actuators)
    int param_len = (register_size + 1) * (number_of_motors) + 2;
    unsigned char params[param_len];

    params[0] = address;
    params[1] = (unsigned char)(register_size & 0xFF);

    int idx = 2;

    // create params from register size
    for (int i = 0; i < number_of_motors; i++) {
        params[idx] = ids[i]; idx++;
        unsigned int curr_value = values[i];
        for (int j = 0; j < register_size; i++) {
            params[i] = (unsigned char)(curr_value & 0xFF); idx++;
            curr_value >>= 8;
        }
    }

    response resp = send_instruction(connection, BROADCAST_ADDRESS, INST_SYNC_WRITE, params, param_len);
    return resp.valid && (resp.error == 0);
}

bool dxl_read_register(int connection, unsigned char id, unsigned char address,
                   unsigned int *result, int register_size) {
    unsigned char params[2];
    params[0] = address;
    params[1] = (unsigned char)(register_size & 0xFF);
    
    response resp = send_instruction(connection, id, INST_READ, params, 2);

    if (resp.valid && resp.error == 0 && resp.param_count > 0) {
        unsigned int ret_result = 0x00;
        for (int i = 0; i < resp.param_count; i++) {
            ret_result += ret_result | (resp.params[i] << (i * 8));
        }

        *result = ret_result;
        return true;
    }
    return false;
}

bool dxl_ping(int connection, unsigned char id) {
    response resp = send_instruction(connection, id, INST_PING, NULL, 0);
    return resp.valid;
}

bool dxl_set_goal_position(int connection, unsigned char id, uint16_t position) {
    return dxl_write_register(connection, id, REG_GOAL_POSITION, position, REG_GOAL_POSITION_SIZE);
}

bool dxl_set_goal_positions_many(int connection, unsigned char *ids, const uint16_t *positions, int number_of_motors) {
    return dxl_sync_write(connection, ids, REG_GOAL_POSITION, number_of_motors, positions, REG_GOAL_POSITION_SIZE);
}

bool dxl_set_moving_speed(int connection, unsigned char id, uint16_t speed) {
    return dxl_write_register(connection, id, REG_MOVING_SPEED, speed, REG_GOAL_POSITION_SIZE);
}

bool dxl_is_moving(int connection, unsigned char id) {
    unsigned int moving_status = 0;
    if (dxl_read_register(connection, id, REG_MOVING, &moving_status, REG_MOVING_SIZE)) {
        return (moving_status == 0x01);
    }
    return false;  // Assume stopped if read fails
}

void dxl_wait_until_stopped(int connection, unsigned char id) {
    while (dxl_is_moving(connection, id)) {
        usleep(100000);  // 100ms polling interval
    }
}
