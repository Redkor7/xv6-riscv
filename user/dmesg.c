#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

static char buf[DMESG_SIZE + 1];

int main(void) {
  int len = dmesg(buf, sizeof(buf));
  if (len < 0) {
    fprintf(2, "dmesg: sys call failed\n");
    exit(1);
  }
  
  printf("%s", buf);
  exit(0);
}