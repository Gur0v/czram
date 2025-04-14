#include "czram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void remove_zram(const char *arg) {
    check_root_privileges();

    if (!strcmp(arg, "--all")) {
        char *output = execute_command_with_output("zramctl --output=NAME --noheadings");
        if (!output) print_error_and_exit("Failed to list zram devices");

        char *dev = strtok(output, "\n");
        while (dev) {
            if (execute_command("swapoff /dev/%s && zramctl --reset /dev/%s", dev, dev) != 0) {
                free(output);
                print_error_and_exit("Failed to remove zram device");
            }
            printf("Removed /dev/%s\n", dev);
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

void list_zram(void) {
    if (execute_command("zramctl") != 0) {
        print_error_and_exit("Failed to list zram devices");
    }
}
