#include <stdio.h>
#include <time.h>
#include "terminal_gfx.h"

double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void tg_print_progress(int current_bytes, int total_bytes) {
    int bar_width = 35;

    static double start_time = -1.0;
    if (start_time < 0.0) {
        start_time = get_time_sec();
    }

    if (total_bytes <= 0) 
        total_bytes = 1;

    double now = get_time_sec();
    double elapsed = now - start_time;

    float ratio = (float)current_bytes / total_bytes;
    if (ratio > 1.0f) 
        ratio = 1.0f;

    double eta = 0.0;
    if (current_bytes > 0) {
        double bytes_per_sec = current_bytes / elapsed;
        int remaining = total_bytes - current_bytes;
        eta = (bytes_per_sec > 0) ? (remaining / bytes_per_sec) : 0.0;
    }

    int pos = (int)(ratio * bar_width);

    printf("\r\033[2K");

    printf("[");

    for (int i = 0; i < bar_width; i++) {
        if (i < pos)
            printf("=");
        else if (i == pos)
            printf(">");
        else
            printf(" ");
    }

    printf("] %3d%% (%d/%d) ",
           (int)(ratio * 100),
           current_bytes,
           total_bytes);

    printf("ETA: %.1fs", eta);

    fflush(stdout);
}
