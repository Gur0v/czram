#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define VERSION "1.2.0 - stable"

void usage();
void version();
int run_command(const char *command);
void check_zramctl();
void ensure_zram_module();
int is_valid_size(const char *size);
int is_valid_algorithm(const char *algo);
void list_zram();
void create_zram(int argc, char **argv);
void remove_zram(const char *arg);
void handle_old_syntax(const char *old_cmd);

void print_error_and_exit(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void check_root_privileges() {
    if (geteuid() != 0) {
        print_error_and_exit("Error: This operation requires root privileges.");
    }
}

void validate_device(const char *device) {
    struct stat st;
    if (stat(device, &st) || !S_ISBLK(st.st_mode)) {
        print_error_and_exit("Invalid device specified.");
    }
}

void handle_old_syntax(const char *old_cmd) {
    fprintf(stderr, "Warning: The '%s' command is deprecated since version %s.\n"
                    "Please update to the new syntax:\n"
                    "  mk -> make\n"
                    "  rm -> toss\n"
                    "  ls -> list\n"
                    "This notification will be removed in the next release, and the app will error out.\n",
            old_cmd, VERSION);
}

void usage() {
    printf("czram - A lightweight utility for zram device management\n"
           "Usage:\n"
           "  czram make [-s|--size SIZE] [-a|--algorithm ALGO]  Create a zram device\n"
           "  czram toss [--all | DEVICE]                        Remove zram devices\n"
           "  czram list                                         List active zram devices\n"
           "  czram -v|--version                                 Display version information\n"
           "\nOptions:\n"
           "  -s, --size SIZE       Size of the zram device (default: 4G)\n"
           "  -a, --algorithm ALGO  Compression algorithm (default: zstd)\n"
           "  --all                 Remove all zram devices\n"
           "  DEVICE                Path to a specific zram device (e.g., /dev/zram0)\n"
           "\nExamples:\n"
           "  czram make -s 2G -a lzo\n"
           "  czram toss --all\n"
           "  czram list\n");
    exit(1);
}

void version() {
    printf("czram %s\n", VERSION);
    exit(0);
}

int run_command(const char *command) {
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, "Command failed: %s\n", command);
    }
    return result;
}

void check_zramctl() {
    if (run_command("command -v zramctl >/dev/null 2>&1")) {
        print_error_and_exit("Error: zramctl is not installed.\nInstall it with: apk add util-linux-zramctl");
    }
}

void ensure_zram_module() {
    if (run_command("lsmod | grep -q '^zram'")) {
        printf("Loading zram module...\n");
        if (run_command("modprobe zram")) {
            print_error_and_exit("Failed to load zram module.");
        }
    }
}

int is_valid_size(const char *size) {
    char dummy[2];
    return sscanf(size, "%*[0-9]%1[GMK]", dummy) == 1;
}

int is_valid_algorithm(const char *algo) {
    const char *valid_algos[] = {"zstd", "lzo", "lz4", "deflate"};
    for (size_t i = 0; i < sizeof(valid_algos) / sizeof(valid_algos[0]); i++) {
        if (!strcmp(algo, valid_algos[i])) return 1;
    }
    return 0;
}

void list_zram() {
    check_zramctl();
    printf("Active zram devices:\n");
    if (run_command("zramctl")) {
        print_error_and_exit("Failed to list zram devices.");
    }
}

void create_zram(int argc, char **argv) {
    char size[16] = "4G", algorithm[16] = "zstd";

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--size")) {
            if (++i >= argc || !is_valid_size(argv[i])) {
                print_error_and_exit("Invalid size specified.");
            }
            strncpy(size, argv[i], sizeof(size) - 1);
        } else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--algorithm")) {
            if (++i >= argc || !is_valid_algorithm(argv[i])) {
                print_error_and_exit("Invalid algorithm specified.");
            }
            strncpy(algorithm, argv[i], sizeof(algorithm) - 1);
        } else {
            print_error_and_exit("Unknown option encountered.");
        }
    }

    check_root_privileges();
    check_zramctl();
    ensure_zram_module();

    char command[256], zram_device[64];
    snprintf(command, sizeof(command), "zramctl --find --size %s --algorithm %s", size, algorithm);
    FILE *pipe = popen(command, "r");
    if (!pipe || !fgets(zram_device, sizeof(zram_device), pipe)) {
        pclose(pipe);
        print_error_and_exit("Failed to create zram device.");
    }
    pclose(pipe);

    zram_device[strcspn(zram_device, "\n")] = '\0';
    printf("Created zram device: %s\n", zram_device);

    snprintf(command, sizeof(command), "mkswap %s && swapon %s", zram_device, zram_device);
    if (run_command(command)) {
        print_error_and_exit("Failed to format or enable swap on zram device.");
    }

    printf("Swap enabled on %s.\n", zram_device);
}

void remove_zram(const char *arg) {
    check_root_privileges();

    if (!strcmp(arg, "--all")) {
        FILE *pipe = popen("zramctl | awk 'NR > 1 {print $1}'", "r");
        if (!pipe) {
            perror("popen");
            exit(1);
        }

        char device[64];
        int found = 0;
        while (fgets(device, sizeof(device), pipe)) {
            device[strcspn(device, "\n")] = '\0';
            printf("Removing %s...\n", device);
            char command[128];
            snprintf(command, sizeof(command), "swapoff %s && zramctl --reset %s", device, device);
            if (run_command(command)) {
                pclose(pipe);
                print_error_and_exit("Failed to remove zram device.");
            }
            found = 1;
        }
        pclose(pipe);

        if (!found) {
            printf("No active zram devices found.\n");
        } else {
            printf("All zram devices removed successfully.\n");
        }
    } else {
        validate_device(arg);

        printf("Removing %s...\n", arg);
        char command[128];
        snprintf(command, sizeof(command), "swapoff %s && zramctl --reset %s", arg, arg);
        if (run_command(command)) {
            print_error_and_exit("Failed to remove zram device.");
        }

        printf("%s removed successfully.\n", arg);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) usage();

    if (!strcmp(argv[1], "make")) {
        create_zram(argc - 2, argv + 2);
    } else if (!strcmp(argv[1], "toss")) {
        if (argc < 3) {
            print_error_and_exit("Error: Missing argument for 'toss'.");
        }
        remove_zram(argv[2]);
    } else if (!strcmp(argv[1], "list")) {
        list_zram();
    } else if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        version();
    } else if (!strcmp(argv[1], "mk") || !strcmp(argv[1], "rm") || !strcmp(argv[1], "ls")) {
        handle_old_syntax(argv[1]);
        if (!strcmp(argv[1], "mk")) {
            create_zram(argc - 2, argv + 2);
        } else if (!strcmp(argv[1], "rm")) {
            if (argc < 3) {
                print_error_and_exit("Error: Missing argument for 'rm'.");
            }
            remove_zram(argv[2]);
        } else if (!strcmp(argv[1], "ls")) {
            list_zram();
        }
    } else {
        usage();
    }

    return 0;
}
