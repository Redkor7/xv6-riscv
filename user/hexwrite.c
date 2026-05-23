#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int hex2val(char c) {
  if(c >= '0' && c <= '9') return c - '0';
  if(c >= 'A' && c <= 'F') return c - 'A' + 10;
  if(c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

int main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(2, "Usage: hexwrite <hexstring> <file>\n");
    exit(1);
  }

  char *hex = argv[1];
  int fd = open(argv[2], 1); // O_WRONLY
  if(fd < 0) {
    fprintf(2, "Cannot open %s\n", argv[2]);
    exit(1);
  }

  int len = strlen(hex);
  if(len % 2 != 0) {
    fprintf(2, "Error: hex string must have even length\n");
    exit(1);
  }

  int blen = len / 2;
  char buf[128]; 
  if(blen > sizeof(buf)) {
    fprintf(2, "Error: string too long\n");
    exit(1);
  }

  for(int i = 0; i < blen; i++) {
    int h1 = hex2val(hex[2*i]);
    int h2 = hex2val(hex[2*i+1]);
    if(h1 < 0 || h2 < 0) {
      fprintf(2, "Error: invalid hex character\n");
      exit(1);
    }
    buf[i] = (h1 << 4) | h2;
  }

  int w = write(fd, buf, blen);
  if(w < 0 || w != blen) {
    fprintf(2, "Write error\n");
  } else {
    printf("Wrote %d bytes.\n", w);
  }

  close(fd);
  exit(0);
}