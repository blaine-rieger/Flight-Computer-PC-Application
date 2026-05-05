#ifndef SERIAL_H
#define SERIAL_H

#include <termios.h>
#include <stdio.h>

#define BAUD_RATE B921600

speed_t int_to_baudrate(int baud_arg);  
int serial_init(int *fd, const char *port, speed_t baudrate); 
void serial_close(int fd); 
int get_packet_count(int fd); 
int serial_read_bytes(int fd, void *buf, int size); 
int serial_read(int fd, void *buf, int size); 
int serial_write(int fd, const void *buf, int size); 
int download_packets(int fd, FILE *log_file, int total_packets); 

#endif
