#ifndef DYNAMIXEL_PROTOCOL_H
#define DYNAMIXEL_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t id;
    uint8_t error;
    uint8_t params[32];
    int param_count;
    bool valid;
} response;

/**
 * @brief Sends a raw instruction packet to the Dynamixel bus and parses the response.
 *
 * This function constructs the packet header, calculates the checksum
 * based on the instruction and parameters, transmits the data over the serial
 * connection, and blocks until a status packet is received.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the target motor or broadcast ID.
 * @param instruction The instruction byte (e.g., READ_DATA = 0x02, WRITE_DATA = 0x03).
 * @param params Pointer to the array of parameters for the instruction.
 * @param param_len The number of bytes in the params array.
 * @return A response struct representing the motor's status packet.
 */
response send_instruction(int connection, uint8_t id,
                                   uint8_t instruction,
                                   const uint8_t *params, int param_len);

#endif
