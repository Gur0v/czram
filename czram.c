#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#define VER "1.3.1-stable"

void usage(), version();
void ensure_zram(), list_zram();
void create_zram(int argc, char **argv);
void remove_zram(const char *arg);
void check_root(), check_zramctl();
int is_valid_size(const char *sz), is_valid_algo(const char *algo);
void die(const char *msg);
void check_dev(const char *dev);
int run_cmd(const char *cmd);

void die(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

void check_root() {
    if (geteuid() != 0)
        die("This operation requires root privileges.");
}

void check_dev(const char *dev) {
    struct stat st;
    if (stat(dev, &st) || !S_ISBLK(st.st_mode))
        die("Invalid block device specified.");
}

int run_cmd(const char *cmd) {
    int res = system(cmd);
    if (res != 0)
        fprintf(stderr, "Command failed: %s\n", cmd);
    return res;
}

void check_zramctl() {
    if (run_cmd("command -v zramctl >/dev/null 2>&1"))
        die("zramctl is not installed. Try: apk add util-linux-zramctl");
}

void ensure_zram() {
    if (run_cmd("lsmod | grep -q '^zram'")) {
        puts("Loading zram module...");
        if (run_cmd("modprobe zram"))
            die("Could not load zram module.");
    }
}

int is_valid_size(const char *sz) {
    char unit = '\0';
    return (sscanf(sz, "%*d%c", &unit) == 1) && (unit == 'G' || unit == 'M' || unit == 'K');
}

int is_valid_algo(const char *algo) {
    const char *list[] = {"lzo", "lzo-rle", "lz4", "lz4hc", "zstd", "deflate", "842"};
    for (size_t i = 0; i < sizeof(list)/sizeof(list[0]); ++i)
        if (!strcmp(algo, list[i])) return 1;
    return 0;
}

void usage() {
    puts(
        "czram - Lightweight zram manager\n"
        "Usage:\n"
        "  czram make [-s SIZE] [-a ALGO] [-p PRIO]   Create a zram device\n"
        "  czram toss [--all | DEVICE]                Remove zram devices\n"
        "  czram list                                 List active zram devices\n"
        "  czram -v|--version                         Show version info\n"
        "Options:\n"
        "  -s, --size SIZE     Size of zram device (default: 4G)\n"
        "  -a, --algorithm A   Compression algorithm (default: zstd)\n"
        "  -p, --priority P    Swap priority (0-32767, default: 100)\n"
        "  --all               Remove all zram devices\n"
        "Examples:\n"
        "  czram make -s 2G -a lzo -p 500\n"
        "  czram toss --all\n"
    );
    exit(1);
}

void version() {
    printf("czram %s\n", VER);
    exit(0);
}

void list_zram() {
    check_zramctl();
    puts("Active zram devices:");
    if (run_cmd("zramctl"))
        die("Failed to list zram devices.");
}

void create_zram(int argc, char **argv) {
    char sz[16] = "4G", algo[16] = "zstd";
    int prio = 100;

    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--size")) {
            if (++i >= argc || !is_valid_size(argv[i]))
                die("Invalid size format.");
            strncpy(sz, argv[i], sizeof(sz) - 1);
        } else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--algorithm")) {
            if (++i >= argc || !is_valid_algo(argv[i]))
                die("Invalid compression algorithm.");
            strncpy(algo, argv[i], sizeof(algo) - 1);
        } else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--priority")) {
            if (++i >= argc || sscanf(argv[i], "%d", &prio) != 1 || prio < 0 || prio > 32767)
                die("Invalid priority. Must be 0-32767.");
        } else {
            die("Unknown option.");
        }
    }

    check_root();
    check_zramctl();
    ensure_zram();

    char cmd[256], dev[64];
    snprintf(cmd, sizeof(cmd), "zramctl --find --size %s --algorithm %s", sz, algo);

    FILE *fp = popen(cmd, "r");
    if (!fp || !fgets(dev, sizeof(dev), fp)) {
        if (fp) pclose(fp);
        die("zramctl failed to create device.");
    }
    pclose(fp);

    dev[strcspn(dev, "\n")] = 0;

    printf("Created: %s\n", dev);
    snprintf(cmd, sizeof(cmd), "mkswap %s && swapon -p %d %s", dev, prio, dev);
    if (run_cmd(cmd))
        die("Failed to format or enable swap.");

    printf("Swap enabled on %s (priority %d)\n", dev, prio);
}

void remove_zram(const char *arg) {
    check_root();

    if (!strcmp(arg, "--all")) {
        FILE *fp = popen("zramctl | awk 'NR>1 {print $1}'", "r");
        if (!fp) die("Failed to enumerate devices.");

        char dev[64];
        int removed = 0;

        while (fgets(dev, sizeof(dev), fp)) {
            dev[strcspn(dev, "\n")] = 0;
            printf("Removing %s...\n", dev);

            char cmd[128];
            snprintf(cmd, sizeof(cmd), "swapoff %s && zramctl --reset %s", dev, dev);
            if (run_cmd(cmd)) {
                pclose(fp);
                die("Failed to remove zram device.");
            }
            removed = 1;
        }

        pclose(fp);
        puts(removed ? "All zram devices removed." : "No zram devices found.");
    } else {
        check_dev(arg);
        printf("Removing %s...\n", arg);
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "swapoff %s && zramctl --reset %s", arg, arg);
        if (run_cmd(cmd))
            die("Failed to remove zram device.");
        puts("Device removed.");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) usage();

    if (!strcmp(argv[1], "make")) {
        create_zram(argc - 2, argv + 2);
    } else if (!strcmp(argv[1], "toss")) {
        if (argc < 3) die("Missing argument for 'toss'");
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

