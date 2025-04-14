#include "czram.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

int is_valid_size(const char *size) {
    if (!size || !*size) return 0;
    
    char *end;
    long num = strtol(size, &end, 10);
    if (num <= 0) return 0;
    
    if (*end) {
        if (strlen(end) != 1) return 0;
        switch (toupper(*end)) {
            case 'K': case 'M': case 'G': return 1;
            default: return 0;
        }
    }
    return 1;
}

int is_valid_algorithm(const char *algo) {
    const char *valid[] = {"zstd", "lzo", "lz4", "deflate", "lzo-rle", "842", NULL};
    
    for (int i = 0; valid[i]; i++) {
        if (strcasecmp(algo, valid[i]) == 0) return 1;
    }
    return 0;
}

int is_zram_device(const char *device) {
    if (!device || strncmp(device, "/dev/zram", 9) != 0) return 0;
    
    const char *num = device + 9;
    if (!*num) return 0;
    
    while (*num) {
        if (!isdigit(*num++)) return 0;
    }
    return 1;
}
