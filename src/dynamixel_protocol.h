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

response send_instruction(int connection, uint8_t id,
                                   uint8_t instruction,
                                   const uint8_t *params, int param_len);

#endif