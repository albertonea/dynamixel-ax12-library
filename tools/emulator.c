#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);  // Blocking mode
    if (fd == -1 && errno == ENOENT) {
        int wfd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (wfd == -1) {
            perror("create");
            return 1;
        }
        close(wfd);
        fd = open(argv[1], O_RDONLY);
    }
    if (fd == -1) {
        perror("open");
        return 1;
    }

    lseek(fd, 0, SEEK_END);

    char buf[4096];
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        int ret = select(fd + 1, &rfds, NULL, NULL, NULL);  // Block indefinitely
        if (ret == -1) {
            perror("select");
            break;
        }
        if (FD_ISSET(fd, &rfds)) {
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("%s", buf);
                fflush(stdout);
            } else if (n == 0) {
                // EOF; optional: reopen or continue
                lseek(fd, 0, SEEK_END);
            } else {
                perror("read");
                break;
            }
        }
    }

    close(fd);
    return 0;
}
