#ifndef ROBOTARM_DYNAMIXEL_TRANSLATION_H
#define ROBOTARM_DYNAMIXEL_TRANSLATION_H
#include <stdbool.h>
#include <stdint.h>


bool write_register(
    int connection,
    uint8_t id,
    uint8_t address,
    uint16_t value,
    int register_size
    );

bool sync_write_two_bytes(
    int connection,
    const uint8_t *ids,
    uint8_t address,
    int number_of_motors,
    const uint16_t *values
    );

bool sync_write_byte(
    int connection,
    const uint8_t *ids,
    uint8_t address,
    int number_of_motors,
    const uint8_t *values
);

bool read_register(int connection, uint8_t id, uint8_t address,
                   unsigned int *result, int register_size);
#endif