#include "dynamixel_protocol.h"
#include <dynamixel/dynamixel.h>
#include "dynamixel_constants.h"

// calculates the checksum of the packet
uint8_t calculate_checksum(const uint8_t id, const unsigned char length,
                           const uint8_t instruction,
                           const uint8_t *params, const int param_count) {

    // Checksum = ~(ID + Length + Instruction + Params)
    uint32_t sum = id + length + instruction;
    for (int i = 0; i < param_count; i++) {
        sum += params[i];
    }

    return (uint8_t)~sum;
}

// Convert raw buffer bytes into a response structure
response parse_response(const uint8_t *buffer, int bytes_read) {
    response response = {0};
    response.valid = false;

    // Check for standard packet header (0xFF, 0xFF)
    if (buffer[0] == 0xFF && buffer[1] == 0xFF) {
        response.id = buffer[2];
        const uint8_t rx_length = buffer[3];
        response.error = buffer[4];

        // rx_length includes Error(1) + Params(N) + Checksum(1)
        // Subtract 2 to get just the parameter count
        response.param_count = rx_length - 2;

        // Copy parameters to struct, limited by buffer size
        for (int i = 0; i < response.param_count && i < 32; i++) {
            response.params[i] = buffer[5 + i];
        }

        response.valid = true;
    }

    return response;
}

// Construct the raw byte array to send over serial
void build_packet(const uint8_t id,
    const uint8_t instruction,
    const uint8_t *params,
    const int param_len,
    uint8_t *packet) {

    // Packet Length = Parameter Count + Instruction(1) + Checksum(1)
    uint8_t length = param_len + 2;

    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = id;
    packet[3] = length;
    packet[4] = instruction;

    // Copy parameters into packet
    for (int i = 0; i < param_len; i++) {
        packet[5 + i] = params[i];
    }

    // Add checksum to the end
    packet[5 + param_len] = calculate_checksum(id, length, instruction,
                                                         params, param_len);
}

// function that builds the packet, sends it, and parses the reply
response send_instruction(const int connection, const uint8_t id,
                                   const uint8_t instruction,
                                    const uint8_t *params, const int param_len) {

    // 4 header bytes (FF, FF, ID, Len) + Param Length + 2 footer bytes (Instruction, Checksum)
    int raw_packet_len = 4 + param_len + 2;
    uint8_t packet[raw_packet_len];

    build_packet(id, instruction, params, param_len, packet);

    uint8_t rx_buffer[256];
    int bytes_read = write_to_connection(connection, packet, raw_packet_len,
                                         rx_buffer, 256);

    // Broadcast addresses do not return a response packet
    if (id != BROADCAST_ADDRESS) {
        return parse_response(rx_buffer, bytes_read);
    }

    // Return empty response for broadcast
    response response = {0};
    response.valid = false;
    return response;
}
