#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "ext2_fs.h"

uint32_t get_block_addr(int fd, struct ext2_inode *ino, uint32_t idx, uint32_t bs) {
    uint32_t p = bs / 4;
    uint32_t val;

    if (idx < 12) 
        return le32toh(ino->i_block[idx]);
    idx -= 12;

    if (idx < p) {
        uint32_t blk = le32toh(ino->i_block[12]);
        if (!blk) 
            return 0;
        pread(fd, &val, 4, (off_t)blk * bs + idx * 4);
        return le32toh(val);
    }
    idx -= p;

    if (idx < p * p) {
        uint32_t blk = le32toh(ino->i_block[13]);
        if (!blk) 
            return 0;
        pread(fd, &val, 4, (off_t)blk * bs + (idx / p) * 4);
        blk = le32toh(val);
        if (!blk) 
            return 0;
        pread(fd, &val, 4, (off_t)blk * bs + (idx % p) * 4);
        return le32toh(val);
    }
    idx -= p * p;

    uint32_t blk = le32toh(ino->i_block[14]);
    if (!blk) 
        return 0;
    pread(fd, &val, 4, (off_t)blk * bs + (idx / (p * p)) * 4);
    blk = le32toh(val);
    if (!blk) 
        return 0;
    pread(fd, &val, 4, (off_t)blk * bs + ((idx / p) % p) * 4);
    blk = le32toh(val);
    if (!blk) 
        return 0;
    pread(fd, &val, 4, (off_t)blk * bs + (idx % p) * 4);
    return le32toh(val);
}

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

    uint64_t bytes_left = ((uint64_t)le32toh(inode.i_dir_acl) << 32) | le32toh(inode.i_size);
    
    uint32_t block_idx = 0;
    char *zero_buf = calloc(1, block_size);
    char *data_buf = malloc(block_size);

    while (bytes_left > 0) {
        uint32_t to_write = (bytes_left > block_size) ? block_size : bytes_left;
        uint32_t b_addr = get_block_addr(fd, &inode, block_idx, block_size);
        
        if (b_addr == 0) {
            fwrite(zero_buf, 1, to_write, stdout);
        } else {
            pread(fd, data_buf, block_size, (off_t)b_addr * block_size);
            fwrite(data_buf, 1, to_write, stdout);
        }
        
        bytes_left -= to_write;
        block_idx++;
    }

    free(zero_buf);
    free(data_buf);
    close(fd);
    return 0;
}