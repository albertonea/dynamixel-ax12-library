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
    // Create a buffer for parameters: 1 byte for address + N bytes for data
    int param_len = register_size + 1;
    uint8_t params[param_len];

    // The first parameter in a WRITE instruction is always the target register address
    params[0] = address;

    // Deconstruct the value into bytes
    // params[1] = Low Byte, params[2] = High Byte
    for (int i = 1; i < register_size + 1; i++) {
        params[i] = (uint8_t)(value & 0xFF);
        value >>= 8;
    }

    // Send the packet
    response resp = send_instruction(connection, id, INST_WRITE, params, param_len);

    // Return true only if the packet was valid and the motor reported no errors
    return resp.valid && (resp.error == 0);
}

bool sync_write_two_bytes(
    const int connection,
    const uint8_t *ids,
    const uint8_t address,
    const int number_of_motors,
    const uint16_t *values
    ) {
    // Calculate total packet length for Sync Write:
    // Formula: (Data Length per motor + 1 for ID) * Num Motors + Address byte + length byte
    int param_len = 3 * (number_of_motors) + 2;
    uint8_t params[param_len];

    // Sync Write Header:
    // Starting Address of the register
    params[0] = address;
    // Length of data to be written to each motor (2 bytes)
    params[1] = 0x02;

    int curr_index = 2;

    // Iterate through motors to add ID and Data
    for (int i = 0; i < number_of_motors; i++) {
        // Push Motor ID
        params[curr_index] = ids[i]; curr_index++;

        uint16_t curr_value = values[i];

        // Push 2 bytes of data
        for (int j = 0; j < 2; j++) {
            params[curr_index] = (uint8_t)(curr_value & 0xFF); curr_index++;
            curr_value >>= 8;
        }
    }

    // Send to BROADCAST_ADDRESS because Sync Write controls multiple motors simultaneously
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
    // Calculate total packet length for Sync Write:
    // Formula: (Data Length per motor + 1 for ID) * Num Motors + Address Byte + Length Byte
    int param_len = 2 * (number_of_motors) + 2;
    uint8_t params[param_len];

    // Sync Write Header:
    // Starting Address
    params[0] = address;
    // Length of data (1 byte)
    params[1] = 0x01;

    int curr_index = 2;

    // Iterate through motors to pack ID and Data
    for (int i = 0; i < number_of_motors; i++) {
        // Push Motor ID
        params[curr_index] = ids[i]; curr_index++;
        // Push 1 byte of data
        params[curr_index] = values[i]; curr_index++;
    }

    // Send to BROADCAST_ADDRESS because Sync Write controls multiple motors simultaneously
    response resp = send_instruction(connection, BROADCAST_ADDRESS, INST_SYNC_WRITE, params, param_len);
    return resp.valid && (resp.error == 0);
}

bool read_register(const int connection, const uint8_t id, const uint8_t address,
                   unsigned int *result, const int register_size) {
    uint8_t params[2];

    // READ Instruction parameters:
    // Starting Address to read from
    params[0] = address;
    // Number of bytes to read
    params[1] = register_size;

    response resp = send_instruction(connection, id, INST_READ, params, 2);

    // Check if the response packet is valid and no motor errors occurred
    if (resp.valid && resp.error == 0 && resp.param_count > 0) {
        unsigned int ret_result = 0x00;

        // Reconstruct the multibyte integer from the response array.
        for (int i = 0; i < resp.param_count; i++) {
            // Bitwise OR each byte into result
            ret_result |= resp.params[i] << (i * 8);
        }

        *result = ret_result;
        return true;
    }
    return false;
}
