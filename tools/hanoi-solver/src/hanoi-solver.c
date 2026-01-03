#include <stdio.h>
#include <dynamixel/dynamixel.h>
#include <robotarm.h>

// motors 2, 3, 4 positions for different
// stack heights of the blacks
//{0x0, 0xf8}, {0x01, 0xa4}, {0x01, 0x07}

unsigned int block_height_1[3] = {0x0110, 0x01a2, 0x00f5};
unsigned int block_height_2[3] = {0x011f, 0x01b1, 0x00d5};
unsigned int block_height_3[3] = {0x0136, 0x01b7, 0x00c0};
unsigned int block_height_4[3] = {0x0125, 0x01fc, 0x0089};
unsigned int block_height_5[3] = {0x01ae, 0x0084, 0x0289};

// Settings
// Number of blocks
#define num_of_blocks 4

// Motor 1 setting deciding where to place the stacks
const unsigned int from_location = 0x01bc;
const unsigned int to_location = 0x026b;
const unsigned int aux_location = 0x01fd;

unsigned int stack_locations[3] = {from_location, to_location, aux_location};

int stacks[3] = {num_of_blocks, 0, 0};

// Reading test data from the robot
// typedef struct packet {
//   unsigned char id;
//   unsigned char instruction;
//   unsigned char *parameters;
//   int param_len;
// } packet;
//
// typedef struct buff_wrapper {
//   unsigned char *buff;
//   int buff_length;
//   int bytes_read;
// } buff_wrapper;
//
// packet construct_packet_from_buffer(buff_wrapper *buff) {
//   unsigned char id = buff->buff[2];
//   unsigned char length = buff->buff[3];
//   unsigned char error = buff->buff[4];
//
//   unsigned char params[length];
//   for (int i = 5; i < 5 + length; i++) {
//     params[i] = buff->buff[i];
//   }
//
//   packet p = {id, error, params, length};
//   return p;
// }
//
// packet send_packet(packet *packet, int connection) {
//   int length = (*packet).param_len + 7;
//
//   unsigned char *packet_bytes = malloc(length);
//
//   for (int i = 0; i < 2; i++) {
//     (*packet_bytes) = 0xFF;
//     packet_bytes++;
//   }
//
//   (*packet_bytes) = packet->id;
//   packet_bytes++;
//
//   (*packet_bytes) = (unsigned char)length - 3;
//   packet_bytes++;
//
//   (*packet_bytes) = packet->instruction;
//   packet_bytes++;
//
//   unsigned char sum = 0;
//
//   for (int i = 0; i < packet->param_len; i++) {
//     (*packet_bytes) = packet->parameters[i];
//     sum += packet->parameters[i];
//     packet_bytes++;
//   }
//
//   (*packet_bytes) =
//       ~(packet->id + (unsigned char)length - 3 + packet->instruction + sum);
//
//   int buff_len = 100;
//   unsigned char buff[buff_len];
//
//   int bytes_read =
//       write_to_connection(connection, packet_bytes, length, buff, buff_len);
//
//   buff_wrapper bw = {buff, buff_len, bytes_read};
//
//   return construct_packet_from_buffer(&bw);
// }
//
// void move_to_location(int connection, unsigned char id, unsigned char loc_h,
//                       unsigned char loc_l) {
//
//   unsigned char params[] =
//   { GOAL_POSITION,
//     loc_l,
//     loc_h,
//     0x30, // Moving speed L
//     0x00 // Moving speed H
//   };
//
//   packet p = {id, WRITE_DATA, params, 3};
//
//   send_packet(&p, connection);
//
//   // unsigned char cs = ~(id + 0x07 + 0x03 + 0x1e + loc_l + loc_h + 0x30 + 0x00);
//   //
//   // unsigned char arr[] = {0xff,  0xff,  id,   0x07, 0x03, 0x1e,
//   //                        loc_l, loc_h, 0x30, 0x00, cs};
//   //
//   // int buff_len = 100;
//   // unsigned char buff[buff_len];
//   //
//   // int bytes_read = write_to_connection(connection, arr, 11, buff, buff_len);
// }
//
// void wait_until_done(unsigned char id, int connection) {
//   unsigned char params[] = {MOVING, 0x01};
//   packet p = {id, READ_DATA, params, 2};
//
//   while (true) {
//     packet ret = send_packet(&p, connection);
//     unsigned char active = ret.parameters[0];
//
//     if (active == 0x01) {
//       usleep(100000); // wait 100ms before polling again
//     } else {
//       return;
//     }
//   }
// }

void pinch(int connection) {
    dxl_set_goal_position(connection, 5, 0x0130);
    dxl_wait_until_stopped(connection, 5);
}

void unpinch(int connection) {
    dxl_set_goal_position(connection, 5, 0x01FF);
    dxl_wait_until_stopped(connection, 5);
}

void straighten_arm(int connection) {
    dxl_set_goal_position(connection, 2, 0x01FF);
    dxl_set_goal_position(connection, 3, 0x01FF);
    dxl_wait_until_stopped(connection, 2);
    dxl_wait_until_stopped(connection, 3);
}

void rotate_arm(int connection, unsigned int radians) {
    dxl_set_goal_position(connection, 1, radians);
    dxl_wait_until_stopped(connection, 1);
}

int transition(int connection, unsigned int radians,
               unsigned int height[3]) {
    straighten_arm(connection);

    rotate_arm(connection, radians);

    for (int i = 2; i < 5; i++) {
        dxl_set_goal_position(connection, i, height[i - 2]);
    }

    dxl_wait_until_stopped(connection, 2);
    dxl_wait_until_stopped(connection, 3);
    dxl_wait_until_stopped(connection, 4);

    return 0;
}

void motor_height_position(int stack_h, unsigned int (*height)[3]) {
    printf("setting motor height %d!\n", stack_h);

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
    printf("Stack heights: %d, %d, %d\n", stacks[0], stacks[1], stacks[2]);
    straighten_arm(connection);

    // pickup
    unsigned int height[3] = {0};

    motor_height_position(stacks[from], &height);

    transition(connection, stack_locations[from], height);
    pinch(connection);

    motor_height_position(stacks[to] + 1, &height);
    transition(connection, stack_locations[to], height);
    unpinch(connection);

    update_stacks(from, to);

    straighten_arm(connection);
}

void solve(const int n, const int from_index, const int to_index, const int aux_index, const int connection) {
    if (n == 0) {
        return;
    }
    solve(n - 1, from_index, aux_index, to_index, connection);
    move_block(from_index, to_index, connection);

    solve(n - 1, aux_index, to_index, from_index, connection);
}

int main(int argc, char *argv[]) {
    int connection = open_connection("/tmp/robot", 128);

    straighten_arm(connection);

    solve(num_of_blocks, 0, 1, 2, connection);

    return 0;
}
