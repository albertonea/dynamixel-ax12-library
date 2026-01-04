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
#include <pty.h>


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
        printf("💥Invalid header, expected: %02X %02X (len>5) but got: %02X %02X (len=%d)\n",
            DYNAMIXEL_HEADER1,
            DYNAMIXEL_HEADER2,
            packet[0],
            packet[1],
            len);

        return -1; // Invalid header
    }

    unsigned char id = packet[2];
    unsigned int length = packet[3];
    unsigned char instruction = packet[4];
    unsigned int checksum = packet[len - 1];


    //    unsigned int sum = id + length + instruction;
    // for (int i = 0; i < param_count; i++) {
    //     sum += params[i];
    // }
    // return (unsigned char)~sum;

    // Calculate expected checksum
    unsigned int calc_checksum = id + length + instruction;
    for (int i = 5; i < len - 1; i++) {
        calc_checksum += packet[i];
    }
    calc_checksum = ~calc_checksum & 0xFF;

    if (checksum != calc_checksum) {
        // Send error response (checksum error)
        printf("💥Checksum mismatch, expected: %02X but got: %02X\n", calc_checksum, checksum);

        response[0] = DYNAMIXEL_HEADER1;
        response[1] = DYNAMIXEL_HEADER2;
        response[2] = id;
        response[3] = 0x02; // Length
        response[4] = 0x04; // Checksum error
        response[5] = 0x04; // Checksum
        *resp_len = 6;
        return 0;
    }

    // Generate response based on instruction
    switch (instruction) {
        case DYNAMIXEL_INST_PING:
            printf("DYNAMIXEL_INST_PING\n");
            // Ping response - return model, version, etc.
            response[0] = DYNAMIXEL_HEADER1;
            response[1] = DYNAMIXEL_HEADER2;
            response[2] = id;
            response[3] = 0x07; // Length
            response[4] = 0x00; // Status OK
            response[5] = 0x01; // Model number L
            response[6] = 0x02; // Model number H
            response[7] = 0x01; // Firmware version
            response[8] = 0x00; // ID
            response[9] = 0x00; // Baud rate
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
            printf("DYNAMIXEL_INST_READ\n");

            unsigned char address = packet[5];
            unsigned char reg_len = packet[6];

            response[0] = DYNAMIXEL_HEADER1;
            response[1] = DYNAMIXEL_HEADER2;
            response[2] = id;
            response[3] = reg_len + 2; // Length
            response[4] = 0x00; // Status OK

            // Dummy data - current position ~512 (middle of 0-1023 range)
            if (address == 0x24 && reg_len == 2) {
                // Present position
                response[5] = 0x02; // 512 L
                response[6] = 0x00; // 512 H
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
            printf("❓Unknown instruction\n");
            response[0] = DYNAMIXEL_HEADER1;
            response[1] = DYNAMIXEL_HEADER2;
            response[2] = id;
            response[3] = 0x02;
            response[4] = 0x07; // Instruction error
            response[5] = 0x07;
            *resp_len = 6;
            break;
    }

    return 0;
}

void print_hex(const unsigned char *data, int length) {
    printf("Sent %d bytes:\n", length);
    for (int i = 0; i < length; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    if (length % 8 != 0) {
        printf("\n");
    }
}

void *robot_listener(void *arg) {
    robot_emulator_t *emu = (robot_emulator_t *) arg;

    // Make non-blocking for reliable select
    fcntl(emu->fd, F_SETFL, O_NONBLOCK);

    printf("Robot emulator listening on %s (FD=%d)\n", emu->portname, emu->fd);

    unsigned char buffer[256];
    while (emu->running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(emu->fd, &readfds);

        struct timeval timeout = {0, 100000}; // 100ms
        int maxfd = emu->fd + 1;
        int ret = select(maxfd, &readfds, NULL, NULL, &timeout);
        // printf("select ret=%d errno=%d\n", ret, errno);  // Add after select

        if (ret < 0) {
            perror("select");
            break;
        }
        if (ret > 0 && FD_ISSET(emu->fd, &readfds)) {
            ssize_t len = read(emu->fd, buffer, sizeof(buffer));
            if (len > 0) {
                printf("Received %zd bytes: ", len);
                for (ssize_t i = 0; i < len; i++) printf("%02X ", buffer[i]);
                printf("\n");

                unsigned char response[256];
                int resp_len;
                if (parse_dynamixel_packet(buffer, len, response, &resp_len) == 0) {
                    write(emu->fd, response, resp_len);
                    print_hex(response, resp_len);
                    printf("\n");
                }
            } else if (len == 0) {
                printf("EOF on FD\n");
                break;
            }
        }
    }
    return NULL;
}

int create_emulator_port(const char *portname) {
    // Remove existing port if it exists
    unlink(portname);

    // Create a pseudo-terminal (PTY) pair using openpty()
    int master_fd, slave_fd;
    char slave_name[256];
    char master_name[256];
    if (openpty(&master_fd, &slave_fd, slave_name, NULL, NULL) < 0) {
        perror("openpty");
        return -1;
    }

    ttyname_r(master_fd, master_name, sizeof(master_name));

    // Create symlink to portname
    if (symlink(slave_name, portname) < 0) {
        perror("symlink");
        close(master_fd);
        close(slave_fd);
        return -1;
    }

    // Configure slave as serial port (1M baud)
    struct termios tty;
    tcgetattr(slave_fd, &tty);
    cfmakeraw(&tty); // Byte-level: no ICANON, no ECHO
    cfsetospeed(&tty, B1000000);
    cfsetispeed(&tty, B1000000);
    tty.c_cflag |= (CS8 | CREAD | CLOCAL); // 8N1
    tty.c_cflag &= ~(PARENB | CSTOPB | CRTSCTS);
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tcsetattr(slave_fd, TCSANOW, &tty);

    printf("Created emulator port, master:%s (fd=%d) slave:%s -> %s (fd=%d)\n",
           master_name, master_fd, portname, slave_name, slave_fd);

    robot_emulator = malloc(sizeof(robot_emulator_t));
    robot_emulator->fd = master_fd;
    robot_emulator->running = 1;
    robot_emulator->port_created = 1;
    robot_emulator->portname = strdup(portname);

    if (pthread_create(&robot_emulator->listener_thread, NULL, robot_listener, robot_emulator) != 0) {
        free(robot_emulator->portname);
        free(robot_emulator);
        close(slave_fd);
        unlink(portname);
        close(master_fd);
        return -1;
    }

    return master_fd; // Return master FD for external use if needed
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

int test_tty(char *portname) {
    int masterfd, slavefd;
    char *slavedevice;


    masterfd = posix_openpt(O_RDWR | O_NOCTTY);


    if (masterfd == -1
        || grantpt(masterfd) == -1
        || unlockpt(masterfd) == -1
        || (slavedevice = ptsname(masterfd)) == NULL)
        return -1;


    printf("slave device is: %s\n", slavedevice);


    slavefd = open(slavedevice, O_RDWR | O_NOCTTY);
    if (slavefd < 0)
        return -1;
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
