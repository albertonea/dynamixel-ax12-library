#ifndef DYNAMIXEL_PROTOCOL_H
#define DYNAMIXEL_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>
#define BROADCAST_ADDRESS 0xFE

// --- Instructions ---
#define INST_PING       0x01
#define INST_READ       0x02
#define INST_WRITE      0x03
#define INST_REG_WRITE  0x04
#define INST_ACTION     0x05
#define INST_SYNC_WRITE 0x83

// --- Control Table Addresses ---
#define REG_GOAL_POSITION      0x1E
#define REG_GOAL_POSITION_SIZE 2

#define REG_MOVING_SPEED       0x20
#define REG_MOVING_SPEED_SIZE  2

#define REG_PRESENT_POSITION   0x24
#define REG_PRESENT_POSITION_SIZE  2

#define REG_MOVING             0x2E
#define REG_MOVING_SIZE        1

typedef struct {
   uint8_t address;
   int size;
} dxl_register;

// --- Response Structure ---
typedef struct {
    uint8_t id;
    uint8_t error;
    uint8_t params[32];
    int param_count;
    bool valid;
} response;

// --- Core Protocol Functions ---
response send_instruction(int connection, uint8_t id,
                                   uint8_t instruction,
                                   const uint8_t *params, int param_len);

#endif