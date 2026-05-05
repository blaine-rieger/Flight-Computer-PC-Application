#include <stdint.h>
#include <stddef.h>
#include "crc32.h"

uint32_t crc32_calc(const void *data, size_t len) {
    const uint32_t *buf = (const uint32_t *)data;
    size_t words = len / 4;
    uint32_t crc = 0xFFFFFFFFU;
    while (words--) {
        crc ^= *buf++;
        for (int i = 0; i < 32; i++) {
            if (crc & 0x80000000U)
                crc = (crc << 1) ^ 0x04C11DB7U;
            else
                crc <<= 1;
        }
    }
    return crc;
}
