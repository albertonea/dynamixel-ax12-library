#ifndef ROBOTARM_DYNAMIXEL_TRANSLATION_H
#define ROBOTARM_DYNAMIXEL_TRANSLATION_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Writes a value to a specific register on a single Dynamixel motor.
 *
 * This function handles the creation of the param array for a write instruction
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the Dynamixel motor.
 * @param address The control table address to write to.
 * @param value The value to write into the register.
 * @param register_size The size of the register in bytes (this includes L and H registers).
 * @return true if response contains no errors, false otherwise.
 */
bool write_register(
    int connection,
    uint8_t id,
    uint8_t address,
    uint16_t value,
    int register_size
    );

/**
 * @brief Writes to a 2-byte register on multiple motors simultaneously.
 *
 * Sync write allows you to control multiple motors with a single instruction packet,
 * ensuring they act at the exact same time. This function specifically handles
 * 2-byte data (e.g., Goal Position).
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param ids An array of motor IDs to receive the command.
 * @param address The control table address to write to.
 * @param number_of_motors The number of motors in the arrays.
 * @param values An array of 2-byte values corresponding to each motor ID.
 * @return true if response contains no errors, false otherwise.
 */
bool sync_write_two_bytes(
    int connection,
    const uint8_t *ids,
    uint8_t address,
    int number_of_motors,
    const uint16_t *values
    );

/**
 * @brief Writes to a single byte register on multiple motors simultaneously.
 *
 * Allows you to control multiple motors with a single instruction packet,
 * ensuring they act at the exact same time. This function specifically handles
 * single byte data.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param ids An array of motor IDs to receive the command.
 * @param address The control table address to write to.
 * @param number_of_motors The number of motors in the arrays.
 * @param values An array of byte values corresponding to each motor ID.
 * @return true if response contains no errors, false otherwise.
 */
bool sync_write_byte(
    int connection,
    const uint8_t *ids,
    uint8_t address,
    int number_of_motors,
    const uint8_t *values
);

/**
 * @brief Reads a value from a specific register on a single Dynamixel motor.
 *
 * This function sends a read instruction packet and waits for the status packet
 * response containing the requested data.
 *
 * @param connection The file descriptor or handle for the serial connection.
 * @param id The ID of the Dynamixel motor.
 * @param address The control table address to read from.
 * @param[out] result Pointer to an unsigned int where the read value will be stored.
 * @param register_size The size of the register to read in bytes.
 * @return true if response contains no errors, false otherwise.
 */
bool read_register(
    int connection,
    uint8_t id,
    uint8_t address,
    unsigned int *result,
    int register_size
);

#endif
