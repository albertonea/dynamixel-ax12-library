#include <stdio.h>
#include <string.h>
#include <dynamixel/dynamixel.h>
#include <dynamixel_motor.h>

/*
 * PRE-DEFINED POSITIONS
 * These arrays store the joint angles for motors 2, 3, and 4.
 * They correspond to the physical height required to grab a block
 * depending on the stack heights
 */
uint16_t block_height_1[3] = {0x0110, 0x01a2, 0x00f5}; // Lowest position
uint16_t block_height_2[3] = {0x011f, 0x01b1, 0x00d5};
uint16_t block_height_3[3] = {0x0136, 0x01b7, 0x00c0};
uint16_t block_height_4[3] = {0x0125, 0x01fc, 0x0089}; // Highest position

// --- SETTINGS ---
// Total number of blocks in the puzzle can go up to 4
#define num_of_blocks 3

// Robot configuration
const int num_of_motors = 5;
const uint8_t motor_ids[5] = {1, 2, 3, 4, 5};

/*
 * ARM ROTATION LOCATIONS
 * These values define the angle for the three distinct stacks.
 */
const uint16_t from_location = 0x01bc;
const uint16_t to_location = 0x026b;
const uint16_t aux_location = 0x01fd;

// Array to store motor values of the stack locations by index (0, 1, or 2)
uint16_t radial_stack_locations[3] = {from_location, to_location, aux_location};

// Keeps track of how many blocks are currently on each stack
int stacks[3] = {num_of_blocks, 0, 0};


// Controls Motor 5 to grab blocks.
void pinch(int connection) {
    // releases a block by setting the claw motors value to be open
    // 0x0130 represents the closed position
    dxl_set_goal_position(connection, 5, 0x0130);
    dxl_wait_until_stopped(connection, 5);
}

// Controls Motor 5 to release blocks.
void unpinch(int connection) {
    // releases a block by setting the claw motors value to be open
    // 0x01FF represents the open position
    dxl_set_goal_position(connection, 5, 0x01FF);
    dxl_wait_until_stopped(connection, 5);
}

// Lifts the arm up to a safe position to avoid knocking over stacks
void straighten_arm(int connection) {
    uint8_t ids[2] = {2, 3};              // Using Shoulder and Elbow motors
    uint16_t positions[2] = {0x01FF, 0x01FF}; // Safe vertical position

    dxl_set_goal_position_multi(connection, ids, positions, 2);
    dxl_wait_until_all_stopped(connection, ids, 2);
}

// Rotates the base to a specific angle
void rotate_arm(int connection, uint16_t radians) {
    dxl_set_goal_position(connection, 1, radians);
    dxl_wait_until_stopped(connection, 1);
}

// Executes a full movement sequence
// Up out of the way -> Rotate -> Down to target height
int transition(int connection, uint16_t radians, const uint16_t height[3]) {
    // Move up to not knock over other stacks when rotating
    straighten_arm(connection);

    // Rotate arm to the target stack position
    rotate_arm(connection, radians);

    // Lower arm to the current block height of the stack
    uint8_t ids[3] = {2, 3, 4};
    dxl_set_goal_position_multi(connection, ids, height, 3);
    dxl_wait_until_all_stopped(connection, ids, 3);

    return 0;
}

// Selects the correct joint angles based on the current height of a stack
void motor_height_position(int stack_h, uint16_t (*height)[3]) {
    switch (stack_h) {
        case 1:
            memcpy(*height, block_height_1, sizeof(block_height_1));
            break;
        case 2:
            memcpy(*height, block_height_2, sizeof(block_height_2));
            break;
        case 3:
            memcpy(*height, block_height_3, sizeof(block_height_3));
            break;
        case 4:
            memcpy(*height, block_height_4, sizeof(block_height_4));
            break;
        default:
            printf("Something has gone wrong! Robot attempting to take non-existent block.\n");
    }
}

// Function to update the in memory representation of the blocks
void update_stacks(int from, int to) {
    // Remove from source
    stacks[from] -= 1;

    if (stacks[from] < 0)
        printf("Stack %d is negative!\n", from);

    // Add to destination
    stacks[to] += 1;

    if (stacks[to] > num_of_blocks)
        printf("Stack %d is too high! Stack %d is %d high!\n", to, to, stacks[to]);
}

/*
 * Function that encapsulates the physical movement logic. Takes the location to move a block
 * from and where to place it
 */
void move_block(int from, int to, int connection) {
    printf("Moving block from stack %d to stack %d\n", from, to);

    uint16_t height[3];

    // --- PICKUP SEQUENCE ---
    // Calculate position for the top block of the 'from' stack
    motor_height_position(stacks[from], &height);
    // Rotate arm to 'from' stack then lower arm to the tower height
    transition(connection, radial_stack_locations[from], height);
    // Grab block
    pinch(connection);

    // --- DROPOFF SEQUENCE ---
    // Calculate position for the new height of the 'to' stack (current + 1)
    motor_height_position(stacks[to] + 1, &height);
    // Rotate arm to 'to' stack and lower arm to the block height
    transition(connection, radial_stack_locations[to], height);
    // Release block
    unpinch(connection);

    // Update internal state and reset arm
    update_stacks(from, to);
    straighten_arm(connection);

    printf("Stack heights: %d, %d, %d\n", stacks[0], stacks[1], stacks[2]);
}

/*
 * Towers of Hanoi solution
 *
 * n: number of disks to move
 * from_index, to_index, aux_index: identifiers for the block locations
 */
void solve(const int n, const int from_index, const int to_index, const int aux_index, const int connection) {
    if (n == 0) {
        return; // Base case: no blocks left to move
    }

    // Move n-1 blocks from source to aux location
    solve(n - 1, from_index, aux_index, to_index, connection);

    // Physically move the topmost block from source to destination location
    move_block(from_index, to_index, connection);

    // Move the n-1 blocks from aux to destination location
    solve(n - 1, aux_index, to_index, from_index, connection);
}

// Terminal color codes for output formatting
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"

// Setup function that Checks connections and sets default motor speeds
void initialise(int connection) {
    uint16_t speeds[num_of_motors];

    for (int i = 0; i < num_of_motors; i++) {
        // Ping motor to check if it exists on the bus
        if (!dxl_ping(connection, motor_ids[i])) {
            printf(COLOR_RED "ERROR:" COLOR_RESET " Motor %d not found, bad things may happen\n", motor_ids[i]);
        } else {
            printf(COLOR_GREEN "FOUND:" COLOR_RESET " Motor with id %d, setting speed\n", motor_ids[i]);
        }

        // Set a moderate speed (0x0030) for safety
        speeds[i] = 0x0030;
    }

    // Apply speeds to all motors simultaneously
    dxl_set_moving_speed_multi(connection, motor_ids, speeds, num_of_motors);
}

int main(int argc, char *argv[]) {
    // Check for correct command line usage (needs serial port path)
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/ttyUSB0\n", argv[0]);
        return 1;
    }

    // Establish serial connection and initialise motors
    int connection = open_connection(argv[1], B1000000);
    initialise(connection);

    // Ensure robot starts in a safe position
    straighten_arm(connection);

    // Start the recursive hanoi solver
    solve(num_of_blocks, 0, 1, 2, connection);

    return 0;
}
