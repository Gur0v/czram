/**
 * @file main.c
 * @brief Entry point for czram.
 * 
 * Parses command-line arguments and invokes the appropriate zram management function.
 */

#include "czram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Main entry point of czram.
 * 
 * Parses command-line arguments and calls the corresponding function.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Exit status of the program.
 */
int main(int argc, char **argv) {
    // Ensure at least one argument is provided
    if (argc < 2) {
        usage();
    }

    // Parse the first argument to determine the command
    if (!strcmp(argv[1], "make")) {
        create_zram(argc - 2, argv + 2);
    } else if (!strcmp(argv[1], "toss")) {
        if (argc < 3) {
            print_error_and_exit("Missing argument for 'toss'. Use --all or specify a device.");
        }
        remove_zram(argv[2]);
    } else if (!strcmp(argv[1], "list")) {
        list_zram();
    } else if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        version();
    } else {
        usage();
    }

    return 0;
}

/**
 * @brief Displays usage information for czram.
 */
void usage(void) {
    printf("czram - A lightweight utility for zram device management\n"
           "Usage:\n"
           "  czram make [-s SIZE] [-a ALGO]  Create a zram device\n"
           "  czram toss [--all|DEVICE]       Remove a zram device or all devices\n"
           "  czram list                      List active zram devices\n"
           "  czram -v, --version             Show version\n\n"
           "Options:\n"
           "  -s, --size SIZE      Device size (default: 4G)\n"
           "  -a, --algorithm ALGO Compression algorithm (default: zstd)\n");
    exit(0);
}

/**
 * @brief Displays the version of czram.
 */
void version(void) {
    printf("czram %s\n", VERSION);
    exit(0);
}

/**
 * @brief Prints an error message and exits the program.
 * 
 * @param message The error message to display.
 */
void print_error_and_exit(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

/**
 * @brief Checks if the current user has root privileges.
 * 
 * Exits the program with an error message if not running as root.
 */
void check_root_privileges(void) {
    if (geteuid() != 0) {
        print_error_and_exit("This operation requires root privileges. Please run as root.");
    }
}
