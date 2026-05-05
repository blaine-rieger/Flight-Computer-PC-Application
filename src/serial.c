#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "serial.h"
#include "packet.h"
#include "terminal_gfx.h"

speed_t int_to_baudrate(int baud_arg) {
    switch(baud_arg) {
        case 9600:
            return B9600; 
        case 19200:
            return B19200; 
        case 38400:
            return B38400; 
        case 57600: 
            return B57600; 
        case 115200: 
            return B115200;
        case 921600:
            return B921600; 
        case 230400:
            return B230400; 
        default:
            return (speed_t) - 1; 
    }
}

int serial_init(int *fd, const char *port, speed_t baudrate) {
    // usleep(500000);

    struct termios tty;

    *fd = open(port, O_RDWR | O_NOCTTY);
    if (*fd < 0) {
        perror("\nOpen");
        return -1;
    }

    memset(&tty, 0, sizeof tty);

    if (tcgetattr(*fd, &tty) != 0) {
        perror("tcgetattr");
        close(*fd);
        return -1;
    }

    cfsetispeed(&tty, baudrate);
    cfsetospeed(&tty, baudrate);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag = 0;
    tty.c_iflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 20;

    if (tcsetattr(*fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(*fd);
        return -1;
    }

    return 0;
}

void serial_close(int fd) {
    tcflush(fd, TCIOFLUSH);
    close(fd);
}

int get_packet_count(int fd) {
    char buf[64];
    if (serial_write(fd, "COUNT\n", 6) < 0) {
        printf("Failed to send COUNT\n");
        return -1;
    }

    int idx = 0;
    uint8_t c;
    while (idx < (int)sizeof(buf) - 1) {
        int n = read(fd, &c, 1);
        if (n <= 0) {
            printf("Failed to read count\n");
            return -1;
        }
        if (c == '\n') break;
        if (c == '\r') continue;
        buf[idx++] = c;
    }
    buf[idx] = '\0';

    char *end;
    errno = 0;
    long count = strtol(buf, &end, 10);
    if (end == buf || errno != 0 || count <= 0) {
        printf("Failed to parse packet count\n");
        return -1;
    }
    return (int)count;
}

int serial_read_bytes(int fd, void *buf, int size) {
    int total = 0;

    while (total < size) {
        int n = read(fd, (char*)buf + total, size - total);
        if (n <= 0) { 
            return -1; 
        }
        total += n; 
    }

    return total;
}

int serial_write(int fd, const void *buf, int size) {
    int total_written = 0;

    while (total_written < size) {
        int n = write(fd,
                      (const char *)buf + total_written,
                      size - total_written);

        if (n <= 0) {
            return -1; 
        }

        total_written += n;
    }

    return total_written;
}

int serial_read(int fd, void *buf, int size) {
    int total_read = 0;
    while (total_read < size) {
        int n = read(fd,
                     (char *)buf + total_read,
                     size - total_read);
        if (n <= 0)
            return total_read;
        total_read += n;
    }
    return total_read;
}

int download_packets(int fd, FILE *log_file, int total_packets) {
    flight_packet_t packet;

    for (int i = 0; i < total_packets; i++) {
        int r = serial_read_bytes(fd, &packet, sizeof(packet));
        if (r != sizeof(packet)) {
            if (r < 0)
                printf("\nError: packet %d read failed\n", i);
            else 
                printf("\nError: packet %d incomplete (%d bytes)\n", i, r);
            return i; 
        }

        if (fwrite(&packet, sizeof(packet), 1, log_file) != 1) {
            printf("\nError: failed to write packet %d to log\n", i);
            return i;
        }

        tg_print_progress(i + 1, total_packets);
    }

    return total_packets;
}
