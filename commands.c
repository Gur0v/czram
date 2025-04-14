/**
 * @file commands.c
 * @brief Implementation of zram device management commands.
 * 
 * This file contains functions to create, remove, and list zram devices.
 */

#include "czram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Creates a new zram device with specified size and compression algorithm.
 * 
 * Parses command-line arguments to determine the size and algorithm for the zram device.
 * Checks for root privileges, creates the device using `zramctl`, and activates it as swap.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 */
void create_zram(int argc, char **argv) {
    char size[MAX_SIZE_LEN] = "4G";
    char algorithm[MAX_ALGO_LEN] = "zstd";

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--size")) {
            if (++i >= argc || !is_valid_size(argv[i])) {
                print_error_and_exit("Invalid size specified");
            }
            strncpy(size, argv[i], MAX_SIZE_LEN);
        } 
        else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--algorithm")) {
            if (++i >= argc || !is_valid_algorithm(argv[i])) {
                print_error_and_exit("Invalid algorithm specified");
            }
            strncpy(algorithm, argv[i], MAX_ALGO_LEN);
        }
    }

    check_root_privileges();

    char *zram_dev = execute_command_with_output(
        "zramctl --find --size %s --algorithm %s", size, algorithm);
    if (!zram_dev) print_error_and_exit("Failed to create zram device");

    zram_dev[strcspn(zram_dev, "\n")] = '\0';
    
    if (execute_command("mkswap %s && swapon %s", zram_dev, zram_dev) != 0) {
        free(zram_dev);
        print_error_and_exit("Failed to activate swap");
    }

    printf("Created zram device: %s\n", zram_dev);
    free(zram_dev);
}

/**
 * @brief Removes a zram device or all zram devices.
 * 
 * If the argument is `--all`, removes all active zram devices. Otherwise, removes the specified device.
 * Ensures root privileges are available before proceeding.
 * 
 * @param arg The zram device to remove, or `--all` to remove all devices.
 */
void remove_zram(const char *arg) {
    check_root_privileges();

    if (!strcmp(arg, "--all")) {
        char *output = execute_command_with_output("zramctl --output=NAME --noheadings");
        if (!output) print_error_and_exit("Failed to list zram devices");

        char *dev = strtok(output, "\n");
        while (dev) {
            dev[strcspn(dev, "\n")] = '\0';

            if (!is_zram_device(dev)) {
                free(output);
                print_error_and_exit("Invalid zram device encountered");
            }

            if (execute_command("swapoff %s && zramctl --reset %s", dev, dev) != 0) {
                free(output);
                print_error_and_exit("Failed to remove zram device");
            }
            printf("Removed %s\n", dev);

            dev = strtok(NULL, "\n");
        }
        free(output);
    } 
    else {
        if (!is_zram_device(arg)) {
            print_error_and_exit("Invalid zram device specified");
        }
        
        if (execute_command("swapoff %s && zramctl --reset %s", arg, arg) != 0) {
            print_error_and_exit("Failed to remove zram device");
        }
        printf("Removed %s\n", arg);
    }
}

/**
 * @brief Lists all active zram devices.
 * 
 * Executes the `zramctl` command to display information about active zram devices.
 */
void list_zram(void) {
    if (execute_command("zramctl") != 0) {
        print_error_and_exit("Failed to list zram devices");
    }
}
