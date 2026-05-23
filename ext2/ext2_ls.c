#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "ext2_fs.h"

int main() {
    uint8_t header[8];
    char name[256];
    char dummy_buf[4096];

    printf("%-10s | %s\n", "INODE", "NAME");
    printf("----------------------------------------\n");

    while (fread(header, 1, 8, stdin) == 8) {
        uint32_t inode = le32toh(*(uint32_t*)(header));
        uint16_t rec_len = le16toh(*(uint16_t*)(header + 4));
        uint8_t name_len = header[6];

        if (rec_len < 8) break; 

        if (inode != 0) {
            fread(name, 1, name_len, stdin);
            printf("%-10u | %.*s\n", inode, name_len, name);
            
            int bytes_to_skip = rec_len - 8 - name_len;
            if (bytes_to_skip > 0) {
                fread(dummy_buf, 1, bytes_to_skip, stdin);
            }
        } else {
            int bytes_to_skip = rec_len - 8;
            if (bytes_to_skip > 0) {
                fread(dummy_buf, 1, bytes_to_skip, stdin);
            }
        }
    }

    return 0;
}