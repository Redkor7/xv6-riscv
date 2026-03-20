#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"

const char *state_names[] = {
  [0] "UNUSED", 
  [1] "USED", 
  [2] "SLEEPING", 
  [3] "RUNNABLE", 
  [4] "RUNNING", 
  [5] "ZOMBIE"
};

int main(void) 
{
  int lim = ps_listinfo(0, 0);
  if (lim <= 0) {
    fprintf(2, "Error: failed to get process count\n");
    exit(1);
  }

  struct procinfo *table = malloc(lim * sizeof(struct procinfo));
  if (!table) {
    fprintf(2, "Error: malloc failed\n");
    exit(1);
  }

  int count = ps_listinfo(table, lim);

  if (count < 0) {
    fprintf(2, "Error: syscall error\n");
    free(table);
    exit(1);
  }

  printf("PID\tNAME\t\tSTATE\t\tPPID\tPNAME\n");
  for (int i = 0; i < count; i++) {
    const char *st = (table[i].state >= 0 && table[i].state <= 5) ? state_names[table[i].state] : "UNKNOWN ";
    
    printf("%d\t%s\t\t%s\t%d\t%s\n", 
           table[i].pid, 
           table[i].name, 
           st, 
           table[i].ppid, 
           table[i].pname[0] ? table[i].pname : "-");
  }

  free(table);
  exit(0);
}