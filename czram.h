#ifndef CZRAM_H
#define CZRAM_H

#define VERSION "1.3.0"
#define MAX_SIZE_LEN 16
#define MAX_ALGO_LEN 16
#define MAX_DEVICE_LEN 64
#define MAX_PATH_LEN 256
#define MAX_CMD_LEN 512
#define MAX_OUTPUT_LEN 1024

void usage(void);
void version(void);
void check_root_privileges(void);
void print_error_and_exit(const char *message);

void create_zram(int argc, char **argv);
void remove_zram(const char *arg);
void list_zram(void);

int is_valid_size(const char *size);
int is_valid_algorithm(const char *algo);
int is_zram_device(const char *device);

int execute_command(const char *format, ...);
char *execute_command_with_output(const char *format, ...);

#endif
