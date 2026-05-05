#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint32_t system_time;
	uint8_t flight_status;
	uint8_t reserved_1;
	uint8_t reserved_2;
	uint8_t reserved_3;
	uint16_t cell_1_voltage;
	uint16_t cell_2_voltage;
	int16_t accx, accy, accz;
	int16_t gyrox, gyroy, gyroz;
	int16_t acc2x, acc2y, acc2z;
	int16_t magx, magy, magz;
	int32_t pressure;
	int32_t temp;
	int32_t gpslon;
	int32_t gpslat;
	int32_t gpsheight;
	int32_t gpsspeed;
	uint32_t crc32;
} flight_packet_t;

#endif
