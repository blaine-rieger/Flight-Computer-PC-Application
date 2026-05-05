#ifndef TASK_H
#define TASK_H

#include <stddef.h>

#define MAX_FILES       128
#define MAX_NAME        256
#define DEFAULT_LOG_DIR "logs"
#define DEFAULT_LOG_PATH "./logs"

typedef enum { 
    MODE_NONE, 
    MODE_FLIGHT_CONFIG, 
    MODE_MEMORY_DUMP,
    MODE_BIN_TO_CSV
} run_mode_t;

run_mode_t task_prompt(void);
int prompt_convert_csv(void); 
int prompt_welcome_screen(void); 
int task_flight_config(int fd);
int task_memory_dump(int fd, char *last_log_path, size_t size);
int prompt_bin_to_csv(const char *log_path);
int run_selected_task(int fd);  

#endif
