#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include "serial.h"
#include "task.h"

int main(int argc, char *argv[]) {
    char *device = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
            case 'd':
                device = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -d <device>\n", argv[0]);
                return -1;
        }
    }

    if (!device) {
        fprintf(stderr, "Missing required argument\n"
                        "Usage: %s -d <device>\n", argv[0]);
        return -1;
    }

    if (!prompt_welcome_screen())
        return -1;

    int fd;
    if (serial_init(&fd, device, BAUD_RATE) < 0) {
        fprintf(stderr, "Failed to open %s\n", device);
        fprintf(stderr, "Run: ls /dev/ttyU* /dev/ttyA* to list ports\n");
        return -1;
    }

    printf("\nDevice:   %s\n", device);
    printf("Baudrate: %d\n", BAUD_RATE);

    const int ret = run_selected_task(fd);
    serial_close(fd);
    return ret;
}
