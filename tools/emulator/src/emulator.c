#define _XOPEN_SOURCE 600
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>


// Simple Dynamixel protocol parser/response generator for emulator
#define DYNAMIXEL_HEADER1 0xFF
#define DYNAMIXEL_HEADER2 0xFF
#define DYNAMIXEL_BROADCAST 0xFF
#define DYNAMIXEL_INST_PING 0x01
#define DYNAMIXEL_INST_READ 0x02
#define DYNAMIXEL_INST_WRITE 0x03
#define DYNAMIXEL_INST_STATUS 0x55

typedef struct {
    int fd;
    pthread_t listener_thread;
    int running;
    int port_created;
    char *portname;
} robot_emulator_t;

static robot_emulator_t *robot_emulator = NULL;

// Parse Dynamixel packet and generate response
int parse_dynamixel_packet(unsigned char *packet, int len, unsigned char *response, int *resp_len) {
    if (len < 6 || packet[0] != DYNAMIXEL_HEADER1 || packet[1] != DYNAMIXEL_HEADER2) {
        return -1;  // Invalid header
    }

    unsigned char id = packet[2];
    unsigned char instruction = packet[4];
    unsigned int length = packet[3];
    unsigned int checksum = packet[len-1];

    // Calculate expected checksum
    unsigned int calc_checksum = 0xFF;
    for (int i = 2; i < len-1; i++) {
        calc_checksum += packet[i];
    }
    calc_checksum = ~calc_checksum & 0xFF;

    if (checksum != calc_checksum) {
        // Send error response (checksum error)
        response[0] = DYNAMIXEL_HEADER1;
        response[1] = DYNAMIXEL_HEADER2;
        response[2] = id;
        response[3] = 0x02;  // Length
        response[4] = 0x04;  // Checksum error
        response[5] = 0x04;  // Checksum
        *resp_len = 6;
        return 0;
    }

    // Generate response based on instruction
    switch (instruction) {
        case DYNAMIXEL_INST_PING:
            // Ping response - return model, version, etc.
            response[0] = DYNAMIXEL_HEADER1;
            response[1] = DYNAMIXEL_HEADER2;
            response[2] = id;
            response[3] = 0x07;  // Length
            response[4] = 0x00;  // Status OK
            response[5] = 0x01;  // Model number L
            response[6] = 0x02;  // Model number H
            response[7] = 0x01;  // Firmware version
            response[8] = 0x00;  // ID
            response[9] = 0x00;  // Baud rate
            response[10] = 0x00; // Delay time
            // Checksum
            unsigned int ping_checksum = 0xFF;
            for (int i = 2; i < 10; i++) ping_checksum += response[i];
            response[11] = ~ping_checksum & 0xFF;
            *resp_len = 12;
            break;

        case DYNAMIXEL_INST_READ:
            // Read response - return dummy position data
            {
                unsigned char address = packet[5];
                unsigned char reg_len = packet[6];

                response[0] = DYNAMIXEL_HEADER1;
                response[1] = DYNAMIXEL_HEADER2;
                response[2] = id;
                response[3] = reg_len + 2;  // Length
                response[4] = 0x00;  // Status OK

                // Dummy data - current position ~512 (middle of 0-1023 range)
                if (address == 0x24 && reg_len == 2) {  // Present position
                    response[5] = 0x02;  // 512 L
                    response[6] = 0x00;  // 512 H
                } else {
                    memset(response + 5, 0x00, reg_len);
                }

                // Calculate checksum
                unsigned int read_checksum = 0xFF;
                for (int i = 2; i < reg_len + 5; i++) read_checksum += response[i];
                response[reg_len + 5] = ~read_checksum & 0xFF;
                *resp_len = reg_len + 6;
            }
            break;

        default:
            // Unknown instruction - error response
            response[0] = DYNAMIXEL_HEADER1;
            response[1] = DYNAMIXEL_HEADER2;
            response[2] = id;
            response[3] = 0x02;
            response[4] = 0x07;  // Instruction error
            response[5] = 0x07;
            *resp_len = 6;
            break;
    }

    return 0;
}

void *robot_listener(void *arg) {
    robot_emulator_t *emu = (robot_emulator_t *)arg;
    unsigned char buffer[256];
    unsigned char response[256];

    printf("Robot emulator listening on %s\n", emu->portname);

    while (emu->running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(emu->fd, &readfds);

        struct timeval timeout = {0, 100000};  // 100ms
        int ret = select(emu->fd + 1, &readfds, NULL, NULL, &timeout);

        if (ret > 0 && FD_ISSET(emu->fd, &readfds)) {
            int len = read(emu->fd, buffer, sizeof(buffer));
            if (len > 0) {
                printf("Received %d bytes: ", len);
                for (int i = 0; i < len; i++) printf("%02X ", buffer[i]);
                printf("\n");

                int resp_len = 0;
                if (parse_dynamixel_packet(buffer, len, response, &resp_len) == 0) {
                    write(emu->fd, response, resp_len);
                    printf("Sent response (%d bytes): ", resp_len);
                    for (int i = 0; i < resp_len; i++) printf("%02X ", response[i]);
                    printf("\n");
                }
            }
        }
    }

    return NULL;
}

int create_emulator_port(const char *portname) {
    // Remove existing port if it exists
    unlink(portname);

    // Create a pseudo-terminal (PTY) pair
    int master, slave;
    char slave_name[256];

    master = posix_openpt(O_RDWR | O_NOCTTY);
    if (master < 0) {
        perror("posix_openpt");
        return -1;
    }

    if (grantpt(master) < 0 || unlockpt(master) < 0) {
        close(master);
        return -1;
    }

    // Get slave name
    ptsname_r(master, slave_name, sizeof(slave_name));

    // Create symlink to /dev/ttyUSB0
    if (symlink(slave_name, portname) < 0) {
        perror("symlink");
        close(master);
        return -1;
    }

    // Open slave end for emulator
    slave = open(slave_name, O_RDWR | O_NOCTTY);
    if (slave < 0) {
        perror("open slave");
        unlink(portname);
        close(master);
        return -1;
    }

    // Configure slave as serial port (1M baud)
    struct termios tty;
    tcgetattr(slave, &tty);
    cfsetospeed(&tty, B1000000);
    cfsetispeed(&tty, B1000000);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;
    tcsetattr(slave, TCSANOW, &tty);

    printf("Created emulator port: %s -> %s (master=%d, slave=%d)\n",
           portname, slave_name, master, slave);

    robot_emulator = malloc(sizeof(robot_emulator_t));
    robot_emulator->fd = slave;
    robot_emulator->running = 1;
    robot_emulator->port_created = 1;
    robot_emulator->portname = strdup(portname);

    if (pthread_create(&robot_emulator->listener_thread, NULL, robot_listener, robot_emulator) != 0) {
        free(robot_emulator->portname);
        free(robot_emulator);
        close(slave);
        unlink(portname);
        close(master);
        return -1;
    }

    return master;  // Return master FD (not used by client)
}

void cleanup_emulator() {
    if (robot_emulator) {
        robot_emulator->running = 0;
        pthread_join(robot_emulator->listener_thread, NULL);

        if (robot_emulator->port_created && robot_emulator->portname) {
            unlink(robot_emulator->portname);
            printf("Cleaned up emulator port: %s\n", robot_emulator->portname);
        }

        free(robot_emulator->portname);
        free(robot_emulator);
        robot_emulator = NULL;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/ttyUSB0\n", argv[0]);
        fprintf(stderr, "Creates a robot arm emulator on the specified port\n");
        return 1;
    }

    const char *portname = argv[1];
    printf("Starting robot arm emulator on %s (Ctrl+C to stop)\n", portname);

    int master_fd = create_emulator_port(portname);
    if (master_fd < 0) {
        fprintf(stderr, "Failed to create emulator port\n");
        return 1;
    }

    // Keep running
    printf("Emulator ready! Your robot arm program can now connect to %s\n", portname);
    printf("Press Ctrl+C to stop the emulator\n\n");

    while (1) {
        sleep(1);
    }

    // Cleanup on exit
    cleanup_emulator();
    close(master_fd);
    return 0;
}
