#include "dynamixel_translation.h"

#include <stdbool.h>
#include <stdint.h>

#include "dynamixel_constants.h"
#include "dynamixel_protocol.h"

bool write_register(
    const int connection,
    const uint8_t id,
    const uint8_t address,
    uint16_t value,
    const int register_size
    ) {
    uint8_t params[register_size + 1];
    params[0] = address;

    // create params from register size
    for (int i = 1; i < register_size + 1; i++) {
        params[i] = (uint16_t)(value & 0xFF);
        value >>= 8;
    }

    response resp = send_instruction(connection, id, INST_WRITE, params, 3);
    return resp.valid && (resp.error == 0);
}

bool sync_write_two_bytes(
    const int connection,
    const uint8_t *ids,
    const uint8_t address,
    const int number_of_motors,
    const uint16_t *values
    ) {
    // (L + 1) * N + 2 (L: Data length for each Dynamixel actuator, N: The number of Dynamixel actuators)
    int param_len = 3 * (number_of_motors) + 2;
    uint8_t params[param_len];

    params[0] = address;
    params[1] = (uint8_t)(2 & 0xFF);

    int idx = 2;

    // create params from register size
    for (int i = 0; i < number_of_motors; i++) {
        params[idx] = ids[i]; idx++;
        uint16_t curr_value = values[i];
        for (int j = 0; j < 2; j++) {
            params[idx] = (uint8_t)(curr_value & 0xFF); idx++;
            curr_value >>= 8;
        }
    }

    response resp = send_instruction(connection, BROADCAST_ADDRESS, INST_SYNC_WRITE, params, param_len);
    return resp.valid && (resp.error == 0);
}

bool sync_write_byte(
    const int connection,
    const uint8_t *ids,
    const uint8_t address,
    const int number_of_motors,
    const uint8_t *values
    ) {
    // (L + 1) * N + 2 (L: Data length for each Dynamixel actuator, N: The number of Dynamixel actuators)
    int param_len = 2 * (number_of_motors) + 2;
    uint8_t params[param_len];

    params[0] = address;
    params[1] = (uint8_t)(1 & 0xFF);

    int idx = 2;

    // create params from register size
    for (int i = 0; i < number_of_motors; i++) {
        params[idx] = ids[i]; idx++;
        params[idx] = values[i]; idx++;
    }

    response resp = send_instruction(connection, BROADCAST_ADDRESS, INST_SYNC_WRITE, params, param_len);
    return resp.valid && (resp.error == 0);
}

bool read_register(const int connection, const uint8_t id, const uint8_t address,
                   unsigned int *result, const int register_size) {
    uint8_t params[2];
    params[0] = address;
    params[1] = (uint8_t)(register_size & 0xFF);

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
