#include <stdio.h>
#include <dynamixel/dynamixel.h>
#include <robotarm.h>

// motors 2, 3, 4 positions for different
// stack heights of the blacks
//{0x0, 0xf8}, {0x01, 0xa4}, {0x01, 0x07}

uint16_t block_height_1[3] = {0x0110, 0x01a2, 0x00f5};
uint16_t block_height_2[3] = {0x011f, 0x01b1, 0x00d5};
uint16_t block_height_3[3] = {0x0136, 0x01b7, 0x00c0};
uint16_t block_height_4[3] = {0x0125, 0x01fc, 0x0089};
uint16_t block_height_5[3] = {0x01ae, 0x0084, 0x0289};

// Settings
// Number of blocks
#define num_of_blocks 3

const int num_of_motors = 5;
const uint8_t motor_ids[5] = {1, 2, 3, 4, 5};

// Motor 1 setting deciding where to place the stacks
const uint16_t from_location = 0x01bc;
const uint16_t to_location = 0x026b;
const uint16_t aux_location = 0x01fd;

uint16_t radial_stack_locations[3] = {from_location, to_location, aux_location};

int stacks[3] = {num_of_blocks, 0, 0};

void pinch(int connection) {
    dxl_set_goal_position(connection, 5, 0x0130);
    dxl_wait_until_stopped(connection, 5);
}

void unpinch(int connection) {
    dxl_set_goal_position(connection, 5, 0x01FF);
    dxl_wait_until_stopped(connection, 5);
}

void straighten_arm(int connection) {
    uint8_t ids[2] = {2, 3};
    uint16_t positions[2] = {0x01FF, 0x01FF};

    dxl_set_goal_position_multi(connection, ids, positions, 2);
    dxl_wait_until_all_stopped(connection, ids, 2);
}

void rotate_arm(int connection, uint16_t radians) {
    dxl_set_goal_position(connection, 1, radians);
    dxl_wait_until_stopped(connection, 1);
}

int transition(int connection, uint16_t radians,
               const uint16_t height[3]) {
    straighten_arm(connection);

    rotate_arm(connection, radians);

    uint8_t ids[3] = {2, 3, 4};

    dxl_set_goal_position_multi(connection, ids, height, 3);
    dxl_wait_until_all_stopped(connection, ids, 3);

    return 0;
}

void motor_height_position(int stack_h, uint16_t (*height)[3]) {
    switch (stack_h) {
        case 1:
            memcpy(*height, block_height_1, sizeof(block_height_1));
            break;
        case 2:
            //*height = block_height_2;
            memcpy(*height, block_height_2, sizeof(block_height_2));
            break;
        case 3:
            //*height = block_height_3;
            memcpy(*height, block_height_3, sizeof(block_height_3));
            break;
        case 4:
            //*height = block_height_4;
            memcpy(*height, block_height_4, sizeof(block_height_4));
            break;
        case 5:
            //*height = block_height_5;
            memcpy(*height, block_height_5, sizeof(block_height_5));
            break;
        default:
            printf("Something has gone wrong! Robot attempting to take non-existent "
                "block.\n");
    }
}

void update_stacks(int from, int to) {
    stacks[from] -= 1;

    if (stacks[from] < 0)
        printf("Stack %d is negative!\n", from);

    stacks[to] += 1;

    if (stacks[to] > num_of_blocks)
        printf("Stack %d is too high! Stack %d is %d high!\n", to, to, stacks[to]);
}

void move_block(int from, int to, int connection) {
    printf("Moving block from stack %d to stack %d\n", from, to);
    straighten_arm(connection);

    // pickup height
    uint16_t height[3];

    motor_height_position(stacks[from], &height);

    transition(connection, radial_stack_locations[from], height);
    pinch(connection);

    motor_height_position(stacks[to] + 1, &height);
    transition(connection, radial_stack_locations[to], height);
    unpinch(connection);

    update_stacks(from, to);

    straighten_arm(connection);

    printf("Stack heights: %d, %d, %d\n", stacks[0], stacks[1], stacks[2]);
}

void solve(const int n, const int from_index, const int to_index, const int aux_index, const int connection) {
    if (n == 0) {
        return;
    }
    solve(n - 1, from_index, aux_index, to_index, connection);
    move_block(from_index, to_index, connection);

    solve(n - 1, aux_index, to_index, from_index, connection);
}

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"

void initialise(int connection) {
    uint16_t speeds[num_of_motors];

    for (int i = 0; i < num_of_motors; i++) {
        if (!dxl_ping(connection, motor_ids[i])) {
            printf(COLOR_RED "ERROR:" COLOR_RESET " Motor %d not found, bad things may happen\n", motor_ids[i]);
        } else {
            printf(COLOR_GREEN "FOUND:" COLOR_RESET " Motor with id %d, setting speed\n", motor_ids[i]);
        }

        speeds[i] = 0x0030;
    }

    dxl_set_moving_speed_multi(connection, motor_ids, speeds, num_of_motors);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/ttyUSB0\n", argv[0]);
        fprintf(stderr, "Solves the Hanoi Towers problem with the robot arm connected on /dev/ttyUSB0\n");
        return 1;
    }

    int connection = open_connection(argv[1], B1000000);
    initialise(connection);

    straighten_arm(connection);

    solve(num_of_blocks, 0, 1, 2, connection);

    return 0;
}
