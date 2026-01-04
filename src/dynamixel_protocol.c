#include "dynamixel_protocol.h"
#include <dynamixel/dynamixel.h>

unsigned char calculate_checksum(const unsigned char id, const unsigned char length,
                                     const unsigned char instruction,
                                     const unsigned char *params, const int param_count) {
    unsigned int sum = id + length + instruction;
    for (int i = 0; i < param_count; i++) {
        sum += params[i];
    }
    return (unsigned char)~sum;
}

response parse_response(const unsigned char *buffer, int bytes_read) {
    response response = {0};
    response.valid = false;

    if (buffer[0] == 0xFF && buffer[1] == 0xFF) {
        response.id = buffer[2];
        unsigned char rx_length = buffer[3];
        response.error = buffer[4];

        // Extract parameters
        response.param_count = rx_length - 2; // Length includes error + params + checksum
        for (int i = 0; i < response.param_count && i < 32; i++) {
            response.params[i] = buffer[5 + i];
        }

        response.valid = true;
    }

    return response;
}

void build_packet(const unsigned char id,
    const unsigned char instruction,
    const unsigned char *params,
    const int param_len,
    unsigned char *packet) {

    unsigned char length = param_len + 2;

    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = id;
    packet[3] = length;
    packet[4] = instruction;

    for (int i = 0; i < param_len; i++) {
        packet[5 + i] = params[i];
    }

    packet[5 + param_len] = calculate_checksum(id, length, instruction,
                                                         params, param_len);
}

response send_instruction(const int connection, const unsigned char id,
                                   const unsigned char instruction,
                                    const unsigned char *params, const int param_len) {

    int raw_packet_len = 4 + param_len + 2;
    unsigned char packet[raw_packet_len];

    build_packet(id, instruction, params, param_len, packet);

    unsigned char rx_buffer[256];
    int bytes_read = write_to_connection(connection, packet, raw_packet_len,
                                         rx_buffer, 256);
    
    // Parse response only for non-broadcast ids
    if (id != BROADCAST_ADDRESS) {
        return parse_response(rx_buffer, bytes_read);
    }

    response response = {0};
    response.valid = false;
    return response;
}
