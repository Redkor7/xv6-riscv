#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Кастомный вывод в hex, т.к. printf в xv6 не поддерживает форматирование вида %02X
void print_hex(unsigned char c) {
  char hex[] = "0123456789ABCDEF";
  printf("%c%c ", hex[(c >> 4) & 0xF], hex[c & 0xF]);
}

int main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(2, "Usage: hexdump <bytes> <file>\n");
    exit(1);
  }
  
  int n = atoi(argv[1]);
  int fd = open(argv[2], 0); // O_RDONLY
  if(fd < 0) {
    fprintf(2, "Cannot open %s\n", argv[2]);
    exit(1);
  }

  char buf[32];
  int read_bytes = 0;
  
  while(read_bytes < n) {
    int to_read = (n - read_bytes > sizeof(buf)) ? sizeof(buf) : (n - read_bytes);
    int r = read(fd, buf, to_read);
    if(r < 0) {
      fprintf(2, "Read error\n");
      break;
    }
    if(r == 0) break; // EOF
    
    for(int i = 0; i < r; i++) {
      print_hex((unsigned char)buf[i]);
    }
    read_bytes += r;
  }
  printf("\n");
  close(fd);
  exit(0);
}