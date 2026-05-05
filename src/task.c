#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "serial.h"
#include "terminal_gfx.h"
#include "json_config.h"
#include "dirent.h"

static int is_bin_file(const char *name) {
    const char *ext = strrchr(name, '.');
    return (ext && strcmp(ext, ".bin") == 0);
}

static int read_line(char *buf, size_t size) {
    if (!fgets(buf, size, stdin)) return -1;
    buf[strcspn(buf, "\n")] = '\0';
    return 0;
}

static void print_file_size(long bytes) {
    if (bytes > 1000000)
        printf("File size: %.2f MB\n", bytes / 1000000.0);
    else
        printf("File size: %.2f KB\n", bytes / 1000.0);
}

static void convert_file(const char *dir, const char *name) {
    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);
    printf("Converting: %s\n", fullpath);
    // TODO
    // convert_bin_to_csv(fullpath);
}

static int scan_bin_files(const char *log_path,
                          char files[][MAX_NAME], int max) {
    DIR *dir = opendir(log_path);
    if (!dir) { 
        perror("opendir"); 
        return -1; 
    }

    int file_count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && file_count < max) {
        if (!is_bin_file(entry->d_name)) 
            continue;
        snprintf(files[file_count], MAX_NAME, "%s", entry->d_name);
        file_count++;
    }

    closedir(dir);

    return file_count;
}

static int prompt_file_choice(char files[][MAX_NAME], int file_count) {
    printf("\nSelect which file you want to encode to CSV:\n");
    for (int i = 0; i < file_count; i++)
        printf("  [%d] %s\n", i + 1, files[i]);
    printf("  [%d] Convert all BIN files in the directory.\n", file_count + 1);
    printf("> ");
    fflush(stdout);

    char line[16];
    if (read_line(line, sizeof(line)) < 0) return -1;

    char *end;
    long selection = strtol(line, &end, 10);
    if (end == line || selection < 1 || selection > file_count + 1) {
        printf("Invalid selection\n");
        return -1;
    }

    return (selection == file_count + 1) ? 0 : (int)selection;
}

static int prompt_log_filename(char *log_path, size_t size) {
    printf("Enter dump filename: ");
    fflush(stdout);

    char name[256];
    if (read_line(name, sizeof(name)) < 0) return -1;

    if (name[0] == '\0') {
        fprintf(stderr, "Filename cannot be empty\n");
        return -1;
    }

    if (strstr(name, "..") || strchr(name, '/')) {
        fprintf(stderr, "Invalid filename: must not contain '..' or '/'\n");
        return -1;
    }

    snprintf(log_path, size, "%s/%s", DEFAULT_LOG_DIR, name);
    return 0;
}

run_mode_t task_prompt(void) {
    printf("\nSelect mode\n");
    printf("  [1] Flight Configuration\n");
    printf("  [2] Memory Dump\n");
    printf("  [3] Bin to CSV\n");
    printf("> ");
    fflush(stdout);

    char line[16];
    if (read_line(line, sizeof(line)) < 0) return MODE_NONE;

    switch (line[0]) {
        case '1': return MODE_FLIGHT_CONFIG;
        case '2': return MODE_MEMORY_DUMP;
        case '3': return MODE_BIN_TO_CSV;
        default:
            fprintf(stderr, "Invalid selection\n");
            return MODE_NONE;
    }
}

int prompt_convert_csv(void) {
    printf("\nConvert dump to CSV? [y/n]: ");
    fflush(stdout);

    char line[8];
    if (read_line(line, sizeof(line)) < 0) return 0;
    return (line[0] == 'y' || line[0] == 'Y');
}

int prompt_welcome_screen(void) {
    printf("\nEnsure configuration jumper is installed.\n");
    printf("  [1] Continue to connect\n");
    printf("  [2] Cancel\n");
    printf("> ");
    fflush(stdout);

    char line[16];
    if (read_line(line, sizeof(line)) < 0) return -1;

    if (line[0] != '1') {
        printf("Cancelled.\n");
        return 0;
    }

    return 1;
}

int task_flight_config(int fd) {
    printf("\nWARNING: THIS WILL ERASE ENTIRE FLASH MEMORY.\n");
    printf("  [1] Continue and erase\n");
    printf("  [2] Cancel\n");
    printf("> ");
    fflush(stdout);

    char line[16];
    if (read_line(line, sizeof(line)) < 0) return -1;

    if (line[0] != '1') {
        printf("Cancelled.\n");
        return 0;
    }

    if (send_flight_config(fd, "config.json") < 0) {
        fprintf(stderr, "Failed to send flight configuration\n");
        return -1;
    }

    printf("\nFlight configuration complete\n");
    printf("Remove serial connection cable\n");
    printf("\nRocket is armed and ready for launch\n");
    printf("\nRemove before flight tag to initiate pad wait\n");
    return 0;
}

int task_memory_dump(int fd, char *last_log_path, size_t size) {
    if (prompt_log_filename(last_log_path, size) < 0)
        return -1;

    printf("Log file: %s\n", last_log_path);

    int total_packets = get_packet_count(fd);
    if (total_packets <= 0) 
        return -1;
    printf("Flight log contains %d packets\n", total_packets);

    FILE *log_file = fopen(last_log_path, "wb");
    if (!log_file) { 
        perror("fopen log"); 
        return -1; 
    }

    if (serial_write(fd, "DUMP\n", 5) < 0) {
        fprintf(stderr, "Failed to send DUMP command\n");
        fclose(log_file);
        return -1;
    }

    double start_time = get_time_sec();
    int received = download_packets(fd, log_file, total_packets);
    double elapsed = get_time_sec() - start_time;

    fclose(log_file);

    printf("\nPackets received: %d/%d\n", received, total_packets);
    printf("Transfer time: %.2f seconds\n", elapsed);
    print_file_size(total_packets * 64L);

    return 0;
}

int prompt_bin_to_csv(const char *log_path) {
    if (!log_path) { fprintf(stderr, "Invalid log_path\n"); 
        return -1; }

    char files[MAX_FILES][MAX_NAME];
    int file_count = scan_bin_files(log_path, files, MAX_FILES);
    if (file_count < 0)  
        return -1;
    if (file_count == 0) { 
        printf("No .bin files found.\n"); 
        return -1; 
    }

    int selection = prompt_file_choice(files, file_count);
    if (selection < 0) 
        return -1;

    if (selection == 0) {
        for (int i = 0; i < file_count; i++)
            convert_file(log_path, files[i]);
    } else {
        printf("Selected file: %s\n", files[selection - 1]);
        convert_file(log_path, files[selection - 1]);
    }

    return 0;
}

int run_selected_task(int fd) {
    switch (task_prompt()) {
        case MODE_FLIGHT_CONFIG:
            return task_flight_config(fd);

        case MODE_MEMORY_DUMP: {
            char last_log_path[256] = {0};
            int ret = task_memory_dump(fd, last_log_path, sizeof(last_log_path));
            if (ret == 0 && prompt_convert_csv())
                return prompt_bin_to_csv(DEFAULT_LOG_DIR);
            return ret;
        }
    
        case MODE_BIN_TO_CSV:
            return prompt_bin_to_csv(DEFAULT_LOG_DIR);

        default:
            return -1;
    }
}
