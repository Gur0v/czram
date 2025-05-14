#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define VER "1.4.1"
#define CONF "/etc/default/czram"

void die(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

int run_cmd(const char *cmd) {
    int res = system(cmd);
    if (res != 0)
        fprintf(stderr, "Command failed: %s\n", cmd);
    return res;
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

int is_valid_size(const char *sz) {
    if (strstr(sz, "%RAM")) return 1;
    char unit = '\0';
    return (sscanf(sz, "%*d%c", &unit) == 1) && (unit == 'G' || unit == 'M' || unit == 'K');
}

int is_valid_algo(const char *algo) {
    const char *list[] = {"lzo", "lzo-rle", "lz4", "lz4hc", "zstd", "deflate", "842"};
    for (size_t i = 0; i < sizeof(list)/sizeof(list[0]); ++i)
        if (!strcmp(algo, list[i])) return 1;
    return 0;
}

unsigned long get_total_ram_kb() {
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) die("Cannot read /proc/meminfo");
    unsigned long kb = 0;
    fscanf(f, "MemTotal: %lu", &kb);
    fclose(f);
    return kb;
}

unsigned long parse_size(const char *sz) {
    if (strstr(sz, "%RAM")) {
        int percent = atoi(sz);
        if (percent <= 0 || percent > 100)
            die("Invalid RAM percentage");
        return (get_total_ram_kb() * percent / 100) * 1024;
    }
    unsigned long val;
    char unit;
    if (sscanf(sz, "%lu%c", &val, &unit) != 2)
        die("Invalid size format");
    switch (unit) {
        case 'G': return val * 1024UL * 1024UL * 1024UL;
        case 'M': return val * 1024UL * 1024UL;
        case 'K': return val * 1024UL;
        default: die("Unknown size unit");
    }
    return 0;
}

void load_config(char *sz, char *algo, int *prio) {
    FILE *f = fopen(CONF, "r");
    if (!f) die("Could not read config file");
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || !strchr(line, '=')) continue;
        char *key = strtok(line, "=\n");
        char *val = strtok(NULL, "=\n");
        if (!key || !val) continue;
        if (!strcmp(key, "SIZE")) strncpy(sz, val, 15);
        else if (!strcmp(key, "ALGO")) strncpy(algo, val, 15);
        else if (!strcmp(key, "PRIO")) *prio = atoi(val);
    }
    fclose(f);
}

void ensure_zram() {
    if (run_cmd("lsmod | grep -q '^zram'"))
        if (run_cmd("modprobe zram"))
            die("Could not load zram module.");
}

void list_zram() {
    if (run_cmd("command -v zramctl >/dev/null 2>&1"))
        die("zramctl is not installed. Try: apk add util-linux-zramctl");
    puts("Active zram devices:");
    if (run_cmd("zramctl"))
        die("Failed to list zram devices.");
}

void remove_dev(const char *dev) {
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "swapoff %s && zramctl --reset %s", dev, dev);
    if (run_cmd(cmd)) die("Failed to remove zram device.");
}

void remove_zram(const char *arg) {
    check_root();
    if (!strcmp(arg, "--all")) {
        FILE *fp = popen("zramctl | awk 'NR>1 {print $1}'", "r");
        if (!fp) die("Failed to enumerate devices.");
        char dev[64];
        int found = 0;
        while (fgets(dev, sizeof(dev), fp)) {
            dev[strcspn(dev, "\n")] = 0;
            printf("Removing %s...\n", dev);
            remove_dev(dev);
            found = 1;
        }
        pclose(fp);
        puts(found ? "All zram devices removed." : "No zram devices found.");
    } else {
        check_dev(arg);
        printf("Removing %s...\n", arg);
        remove_dev(arg);
        puts("Device removed.");
    }
}

void create_zram(void) {
    char sz[16] = "", algo[16] = "";
    int prio = -1;
    load_config(sz, algo, &prio);
    if (!is_valid_size(sz)) die("Invalid size in config");
    if (!is_valid_algo(algo)) die("Invalid algorithm in config");
    if (prio < 0 || prio > 32767) die("Invalid priority in config");
    check_root();
    if (run_cmd("command -v zramctl >/dev/null 2>&1"))
        die("zramctl is not installed. Try: apk add util-linux-zramctl");
    ensure_zram();
    unsigned long bytes = parse_size(sz);
    char cmd[256], dev[64];
    snprintf(cmd, sizeof(cmd), "zramctl --find --size %lu --algorithm %s", bytes, algo);
    FILE *fp = popen(cmd, "r");
    if (!fp) die("Failed to run zramctl.");
    if (!fgets(dev, sizeof(dev), fp)) {
        pclose(fp);
        die("Failed to read device from zramctl.");
    }
    pclose(fp);
    dev[strcspn(dev, "\n")] = 0;
    printf("Created: %s\n", dev);
    snprintf(cmd, sizeof(cmd), "mkswap %s && swapon -p %d %s", dev, prio, dev);
    if (run_cmd(cmd))
        die("Failed to format or enable swap.");
    printf("Swap enabled on %s (priority %d)\n", dev, prio);
}

void show_help(int version) {
    if (version) {
        printf("czram %s\n", VER);
    } else {
        puts("czram - Lightweight zram manager\n"
             "Usage:\n"
             "  czram make                        Create a zram device\n"
             "  czram toss [--all | DEVICE]       Remove zram devices\n"
             "  czram list                        List active zram devices\n"
             "  czram -v|--version                Show version info\n");
    }
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 2) show_help(0);
    if (!strcmp(argv[1], "make")) {
        create_zram();
    } else if (!strcmp(argv[1], "toss")) {
        if (argc < 3) die("Missing argument for 'toss'");
        remove_zram(argv[2]);
    } else if (!strcmp(argv[1], "list")) {
        list_zram();
    } else if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        show_help(1);
    } else {
        show_help(0);
    }
    return 0;
}

