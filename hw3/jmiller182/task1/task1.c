#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <required_address>\n", argv[0]);
        return 1;
    }

    unsigned long addr = strtoul(argv[1], NULL, 16);

    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    char line[1024];
    bool valid = false;

    while (fgets(line, sizeof line, fp)) {
        unsigned long start, end;
        char perms[5];

        if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3) {
            if (addr >= start && addr < end && perms[0] == 'r') {
                valid = true;
                break;
            }
        }
    }
    fclose(fp);

    if (!valid) {
        return -1;
    }

    printf("%02x\n", *(unsigned char *)addr);
    return 0;
}