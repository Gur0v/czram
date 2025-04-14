/**
 * @file czram.h
 * @brief Header file for czram.
 * 
 * Contains macros, function declarations, and constants used across the project.
 */

#ifndef CZRAM_H
#define CZRAM_H

#define VERSION "1.3.0 - stable"      /**< Version of the czram tool. */
#define MAX_SIZE_LEN 16               /**< Maximum length of the size string. */
#define MAX_ALGO_LEN 16               /**< Maximum length of the algorithm string. */
#define MAX_DEVICE_LEN 64             /**< Maximum length of a device name. */
#define MAX_PATH_LEN 256              /**< Maximum length of a file path. */
#define MAX_CMD_LEN 512               /**< Maximum length of a shell command. */
#define MAX_OUTPUT_LEN 1024           /**< Maximum length of command output. */

/**
 * @brief Displays usage information for the czram tool.
 */
void usage(void);

/**
 * @brief Displays the version of the czram tool.
 */
void version(void);

/**
 * @brief Checks if the current user has root privileges.
 * 
 * Exits the program with an error message if not running as root.
 */
void check_root_privileges(void);

/**
 * @brief Prints an error message and exits the program.
 * 
 * @param message The error message to display.
 */
void print_error_and_exit(const char *message);

/**
 * @brief Creates a new zram device.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 */
void create_zram(int argc, char **argv);

/**
 * @brief Removes a zram device or all zram devices.
 * 
 * @param arg The zram device to remove, or `--all` to remove all devices.
 */
void remove_zram(const char *arg);

/**
 * @brief Lists all active zram devices.
 */
void list_zram(void);

/**
 * @brief Validates a size string.
 * 
 * @param size The size string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int is_valid_size(const char *size);

/**
 * @brief Validates a compression algorithm string.
 * 
 * @param algo The algorithm string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int is_valid_algorithm(const char *algo);

/**
 * @brief Checks if a given string is a valid zram device.
 * 
 * @param device The device string to validate.
 * @return 1 if valid, 0 otherwise.
 */
int is_zram_device(const char *device);

/**
 * @brief Executes a shell command.
 * 
 * @param format The format string for the command.
 * @param ... Arguments for the format string.
 * @return Exit status of the command.
 */
int execute_command(const char *format, ...);

/**
 * @brief Executes a shell command and captures its output.
 * 
 * @param format The format string for the command.
 * @param ... Arguments for the format string.
 * @return Pointer to the command output, or NULL on failure.
 */
char *execute_command_with_output(const char *format, ...);

#endif
