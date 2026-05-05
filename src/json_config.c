#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "cJSON.h"
#include "json_config.h"
#include "crc32.h"
#include "serial.h"

#include <termios.h>
#include <unistd.h>

#define F_U32(f) { #f, offsetof(json_parameters_t, f), T_U32 }
#define F_F32(f) { #f, offsetof(json_parameters_t, f), T_F32 }
#define NUM_FIELDS (sizeof(fields) / sizeof(fields[0]))

typedef enum { T_U32, T_F32 } field_type_t;

typedef struct {
    const char  *name;
    size_t       offset;
    field_type_t type;
} field_def_t;

static const field_def_t fields[] = {
    F_F32(test),
    F_U32(a0), F_U32(a1), F_U32(a2), F_U32(a3), F_U32(a4), F_U32(a5), F_U32(a6), F_U32(a7),
    F_U32(b0), F_U32(b1), F_U32(b2), F_U32(b3), F_U32(b4), F_U32(b5), F_U32(b6), F_U32(b7),
    F_U32(c0), F_U32(c1), F_U32(c2), F_U32(c3), F_U32(c4), F_U32(c5), F_U32(c6), F_U32(c7),
    F_U32(d0), F_U32(d1), F_U32(d2), F_U32(d3), F_U32(d4), F_U32(d5), F_U32(d6), F_U32(d7),
    F_U32(e0), F_U32(e1), F_U32(e2), F_U32(e3), F_U32(e4), F_U32(e5), F_U32(e6), F_U32(e7),
    F_U32(f0), F_U32(f1), F_U32(f2), F_U32(f3), F_U32(f4), F_U32(f5), F_U32(f6), F_U32(f7),
    F_U32(g0), F_U32(g1), F_U32(g2), F_U32(g3), F_U32(g4), F_U32(g5), F_U32(g6), F_U32(g7),
    F_U32(h0), F_U32(h1), F_U32(h2), F_U32(h3), F_U32(h4), F_U32(h5),
};


static int parse_packet(const char *json_str, json_parameters_t *pkt) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        const char *err = cJSON_GetErrorPtr();
        fprintf(stderr, "JSON parse error near: %s\n", err ? err : "?");
        return -1;
    }

    memset(pkt, 0, sizeof(*pkt));

    for (size_t i = 0; i < NUM_FIELDS; i++) {
        cJSON *obj = cJSON_GetObjectItem(root, fields[i].name);
        if (!cJSON_IsNumber(obj)) continue;

        uint8_t *dst = (uint8_t *)pkt + fields[i].offset;

        if (fields[i].type == T_U32) {
            uint32_t v = (uint32_t)obj->valuedouble;
            memcpy(dst, &v, sizeof(v));
        } else {
            float v = (float)obj->valuedouble;
            memcpy(dst, &v, sizeof(v));
        }
    }

    pkt->crc32 = crc32_calc(pkt, offsetof(json_parameters_t, crc32));

    cJSON_Delete(root);

    return 0;
}

int load_json_config(const char *json_path, json_parameters_t *pkt) {
    FILE *f = fopen(json_path, "rb");
    if (!f) { perror("fopen"); return -1; }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char *buf = malloc(len + 1);
    if (!buf) { fclose(f); fprintf(stderr, "OOM\n"); return -1; }

    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    int ret = parse_packet(buf, pkt);
    free(buf);

    return ret;
}

int send_flight_config(int fd, const char *path) {
    json_parameters_t pkt;

    if (load_json_config(path, &pkt) < 0) {
        fprintf(stderr, "Failed to load config: %s\n", path);
        return -1;
    }

    if (serial_write(fd, "CONFIG\n", 7) != 7) {
        fprintf(stderr, "Failed to send config command\n");
        return -1;
    }

    if (serial_write(fd, &pkt, sizeof(pkt)) != (int)sizeof(pkt)) {
        fprintf(stderr, "Failed to send config packet\n");
        return -1;
    }

    printf("\nConfig sent (%zu bytes, CRC32 = 0x%08X)\n", sizeof(pkt), pkt.crc32);

    char ack[32] = {0};
    int idx = 0;
    uint8_t c;
    while (idx < (int)sizeof(ack) - 1) {
        int n = read(fd, &c, 1);
        if (n <= 0) break;
        if (c == '\n') break;
        if (c == '\r') continue;
        ack[idx++] = c;
    }
    ack[idx] = '\0';

    if (strcmp(ack, "OK") != 0) {
        printf("\nError!\n");

        return -1; 
    }

    else {
        printf("\nFlight computer recieved config\n");
        printf("CRC Test Passed.\n"); 
        printf("CRC32 = 0x%08X\n", pkt.crc32);
    }

    return 0;
}
