#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if(argc < 3){
    fprintf(2, "Usage: logctl <mask> <duration_ticks>\n");
    fprintf(2, "Masks: SYSCALL=1, INTR=2, PROC=4, EXEC=8 (e.g., 15 for all)\n");
    exit(1);
  }

  int mask = atoi(argv[1]);
  int duration = atoi(argv[2]);

  logctrl(mask, duration);
  printf("Logging mask set to %d for %d ticks\n", mask, duration);
  
  exit(0);
}