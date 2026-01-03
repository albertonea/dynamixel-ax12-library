#include "dynamixel_protocol.h"
#include <dynamixel/dynamixel.h>
#include <string.h>
#include <stdio.h>

unsigned char dxl_calculate_checksum(unsigned char id, unsigned char length,
                                      unsigned char instruction, 
                                      unsigned char *params, int param_count) {
    unsigned int sum = id + length + instruction;
    for (int i = 0; i < param_count; i++) {
        sum += params[i];
    }
    return (unsigned char)~sum;
}

dxl_response dxl_parse_response(unsigned char *buffer, int bytes_read) {
    dxl_response response = {0};
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

packet build_packet(unsigned char id, unsigned char instruction, unsigned char *params, int param_len) {
    // Calculate packet size: Header(2) + ID(1) + Length(1) + Inst(1) + Params(N) + Checksum(1)
    unsigned char length = param_len + 2; // Length field = Instruction + Params + Checksum
    int raw_packet_len = 4 + param_len + 2;  // Total bytes to send

    unsigned char tx_packet[raw_packet_len];  // Stack allocation, no malloc needed

    // Build packet
    tx_packet[0] = 0xFF;
    tx_packet[1] = 0xFF;
    tx_packet[2] = id;
    tx_packet[3] = length;
    tx_packet[4] = instruction;

    // Copy parameters
    for (int i = 0; i < param_len; i++) {
        tx_packet[5 + i] = params[i];
    }

    // Add checksum
    tx_packet[5 + param_len] = dxl_calculate_checksum(id, length, instruction,
                                                         params, param_len);

    packet packet = {tx_packet, raw_packet_len};

    return packet;
}

dxl_response dxl_send_instruction(int connection, unsigned char id,
                                   unsigned char instruction,
                                   unsigned char *params, int param_len) {

    packet packet = build_packet(id, instruction, params, param_len);

    // Send and receive
    unsigned char rx_buffer[256];
    int bytes_read = write_to_connection(connection, packet.data, packet.length,
                                         rx_buffer, 256);
    
    // Parse response only for non-broadcast ids
    if (id != 0xFE && bytes_read >= 6) {
        return dxl_parse_response(rx_buffer, id);
    }

    dxl_response response = {0};
    response.valid = false;
    return response;
}
