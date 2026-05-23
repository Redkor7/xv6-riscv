#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "ext2_fs.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <image/device> <inode_num>\n", argv[0]);
        return 1;
    }

    const char *img_path = argv[1];
    uint32_t target_inode = (uint32_t)atoi(argv[2]);

    int fd = open(img_path, O_RDONLY);
    if (fd < 0) { 
        perror("open"); 
        return 1; 
    }

    struct ext2_super_block sb;
    pread(fd, &sb, sizeof(sb), 1024);

    if (le16toh(sb.s_magic) != 0xEF53) {
        fprintf(stderr, "Error: Not an ext2 filesystem.\n");
        close(fd); 
        return 1;
    }

    uint32_t block_size = 1024 << le32toh(sb.s_log_block_size);
    uint32_t inodes_per_group = le32toh(sb.s_inodes_per_group);
    uint16_t inode_size = le16toh(sb.s_inode_size) ? le16toh(sb.s_inode_size) : 128;

    uint32_t group = (target_inode - 1) / inodes_per_group;
    uint32_t index = (target_inode - 1) % inodes_per_group;

    struct ext2_group_desc gd;
    uint64_t gd_offset = (block_size == 1024) ? 2048 : block_size;
    pread(fd, &gd, sizeof(gd), gd_offset + group * sizeof(gd));

    struct ext2_inode inode;
    uint64_t inode_table_offset = (uint64_t)le32toh(gd.bg_inode_table) * block_size;
    pread(fd, &inode, sizeof(inode), inode_table_offset + index * inode_size);

    uint64_t total_size = ((uint64_t)le32toh(inode.i_dir_acl) << 32) | le32toh(inode.i_size);

    printf("=== Inode %u Metadata ===\n", target_inode);
    printf("Mode:  %06o\n", le16toh(inode.i_mode));
    printf("UID:   %u\n", le16toh(inode.i_uid));
    printf("GID:   %u\n", le16toh(inode.i_gid));
    printf("Size:  %lu bytes\n", total_size);
    printf("Links: %u\n", le16toh(inode.i_links_count));
    printf("Blocks(512b): %u\n", le32toh(inode.i_blocks));
    
    printf("\n=== Block Addresses ===\n");
    for(int i=0; i<12; i++)
        printf("Direct   [%2d]: %u\n", i, le32toh(inode.i_block[i]));
    printf("Single   [12]: %u\n", le32toh(inode.i_block[12]));
    printf("Double   [13]: %u\n", le32toh(inode.i_block[13]));
    printf("Triple   [14]: %u\n", le32toh(inode.i_block[14]));

    close(fd);
    return 0;
}