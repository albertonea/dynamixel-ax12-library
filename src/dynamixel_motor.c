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

    dxl_response resp = dxl_send_instruction(connection, id, INST_WRITE, params, 3);
    return resp.valid && (resp.error == 0);
}

bool dxl_read_register(int connection, unsigned char id, unsigned char address,
                   unsigned char *result, int register_size) {
    unsigned char params[2];
    params[0] = address;
    params[1] = (unsigned char)(register_size & 0xFF);
    
    dxl_response resp = dxl_send_instruction(connection, id, INST_READ, params, 2);

    if (resp.valid && resp.error == 0 && resp.param_count > 0) {
        unsigned char ret_result = 0x00;
        for (int i = 0; i < resp.param_count; i++) {
            ret_result += ret_result | (resp.params[i] << (i * 8));
        }

        *result = ret_result;
        return true;
    }
    return false;
}

bool dxl_set_goal_position(int connection, unsigned char id, uint16_t position) {
    return dxl_write_register(connection, id, REG_GOAL_POSITION, position, REG_GOAL_POSITION_SIZE);
}

bool dxl_set_moving_speed(int connection, unsigned char id, uint16_t speed) {
    return dxl_write_register(connection, id, REG_MOVING_SPEED, speed, REG_GOAL_POSITION_SIZE);
}

bool dxl_is_moving(int connection, unsigned char id) {
    unsigned char moving_status = 0;
    if (dxl_read_register(connection, id, REG_MOVING, &moving_status, REG_MOVING_SIZE)) {
        return (moving_status == 0x01);
    }
    return false;  // Assume stopped if read fails
}

void dxl_wait_until_stopped(int connection, unsigned char id) {
    while (dxl_is_moving(connection, id)) {
        usleep(10000);  // 10ms polling interval
    }
}
