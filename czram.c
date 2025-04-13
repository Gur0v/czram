#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

void usage() {
    printf("czram - A zram management utility\n"
           "Usage:\n"
           "  czram mk [-s|--size SIZE] [-a|--algorithm ALGO]  Create a zram device\n"
           "  czram rm [--all | DEVICE]                       Remove zram devices\n"
           "  czram ls                                        List active zram devices\n"
           "\nOptions:\n"
           "  -s, --size SIZE       Size of the zram device (default: 4G)\n"
           "  -a, --algorithm ALGO  Compression algorithm (default: zstd)\n"
           "  --all                 Remove all zram devices\n"
           "  DEVICE                Path to a specific zram device (e.g., /dev/zram0)\n"
           "\nExamples:\n"
           "  czram mk -s 2G -a lzo\n"
           "  czram rm --all\n"
           "  czram ls\n");
    exit(1);
}

int run_command(const char *command) {
    return system(command) != 0;
}

void check_zramctl() {
    if (run_command("command -v zramctl >/dev/null 2>&1")) {
        fprintf(stderr, "Error: zramctl is not installed.\n"
                        "Install it with: doas apk add util-linux-zramctl\n");
        exit(1);
    }
}

void ensure_zram_module() {
    if (run_command("lsmod | grep -q '^zram'")) {
        printf("Loading zram module...\n");
        if (run_command("doas modprobe zram")) {
            fprintf(stderr, "Failed to load zram module.\n");
            exit(1);
        }
    }
}

void list_zram() {
    check_zramctl();
    printf("Active zram devices:\n");
    if (run_command("zramctl")) {
        fprintf(stderr, "Failed to list zram devices.\n");
        exit(1);
    }
}

void create_zram(int argc, char **argv) {
    const char *size = "4G", *algorithm = "zstd";

    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--size")) {
            size = argv[++i];
        } else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--algorithm")) {
            algorithm = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            usage();
        }
    }

    check_zramctl();
    ensure_zram_module();

    char command[256], zram_device[64];
    snprintf(command, sizeof(command), "doas zramctl --find --size %s --algorithm %s", size, algorithm);
    FILE *pipe = popen(command, "r");
    if (!pipe || !fgets(zram_device, sizeof(zram_device), pipe)) {
        fprintf(stderr, "Failed to create zram device.\n");
        exit(1);
    }
    pclose(pipe);

    zram_device[strcspn(zram_device, "\n")] = '\0';
    printf("Created zram device: %s\n", zram_device);

    snprintf(command, sizeof(command), "doas mkswap %s && doas swapon %s", zram_device, zram_device);
    if (run_command(command)) {
        fprintf(stderr, "Failed to format or enable swap on %s.\n", zram_device);
        exit(1);
    }

    printf("Swap enabled on %s.\n", zram_device);
}

void remove_zram(const char *arg) {
    if (!strcmp(arg, "--all")) {
        FILE *pipe = popen("zramctl | awk 'NR > 1 {print $1}'", "r");
        if (!pipe) {
            fprintf(stderr, "Failed to list zram devices.\n");
            exit(1);
        }

        char device[64];
        int found = 0;
        while (fgets(device, sizeof(device), pipe)) {
            device[strcspn(device, "\n")] = '\0';
            printf("Removing %s...\n", device);
            char command[128];
            snprintf(command, sizeof(command), "doas swapoff %s && doas zramctl --reset %s", device, device);
            if (run_command(command)) {
                fprintf(stderr, "Failed to remove %s.\n", device);
                pclose(pipe);
                exit(1);
            }
            found = 1;
        }
        pclose(pipe);

        if (!found) {
            printf("No active zram devices found.\n");
        } else {
            printf("All zram devices removed.\n");
        }
    } else {
        struct stat st;
        if (stat(arg, &st) || !S_ISBLK(st.st_mode)) {
            fprintf(stderr, "Invalid device: %s\n", arg);
            usage();
        }

        printf("Removing %s...\n", arg);
        char command[128];
        snprintf(command, sizeof(command), "doas swapoff %s && doas zramctl --reset %s", arg, arg);
        if (run_command(command)) {
            fprintf(stderr, "Failed to remove %s.\n", arg);
            exit(1);
        }

        printf("%s removed successfully.\n", arg);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) usage();

    if (!strcmp(argv[1], "mk")) {
        create_zram(argc - 2, argv + 2);
    } else if (!strcmp(argv[1], "rm")) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing argument for 'rm'.\n");
            usage();
        }
        remove_zram(argv[2]);
    } else if (!strcmp(argv[1], "ls")) {
        list_zram();
    } else {
        usage();
    }

    return 0;
}
