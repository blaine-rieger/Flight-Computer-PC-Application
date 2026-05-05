#ifndef JSON_CONFIG_H
#define JSON_CONFIG_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    float test;
    uint32_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint32_t b0, b1, b2, b3, b4, b5, b6, b7;
    uint32_t c0, c1, c2, c3, c4, c5, c6, c7;
    uint32_t d0, d1, d2, d3, d4, d5, d6, d7;
    uint32_t e0, e1, e2, e3, e4, e5, e6, e7;
    uint32_t f0, f1, f2, f3, f4, f5, f6, f7;
    uint32_t g0, g1, g2, g3, g4, g5, g6, g7;
    uint32_t h0, h1, h2, h3, h4, h5; 
    uint32_t crc32;
} json_parameters_t;

int load_json_config(const char *json_path, json_parameters_t *pkt);
int send_flight_config(int fd, const char *path);

#endif
