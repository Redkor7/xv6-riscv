CC = gcc
CFLAGS = -Wall -Wextra -g -D_FILE_OFFSET_BITS=64
TARGETS = ext2_info ext2_cat ext2_ls

all: $(TARGETS)

ext2_info: ext2_info.c ext2_fs.h
	$(CC) $(CFLAGS) -o $@ $<

ext2_cat: ext2_cat.c ext2_fs.h
	$(CC) $(CFLAGS) -o $@ $<

ext2_ls: ext2_ls.c ext2_fs.h
	$(CC) $(CFLAGS) -o $@ $<

test: all
	VALGRIND_CMD="valgrind --error-exitcode=1 --leak-check=full -q" bash test.sh

clean:
	rm -f $(TARGETS) test_large test_sparse