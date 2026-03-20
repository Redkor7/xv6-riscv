#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"

void assert_test(int condition, const char *msg) {
  if (!condition) {
    fprintf(2, "FAIL: %s\n", msg);
    exit(1);
  }
  printf("PASS: %s\n", msg);
}

int main(void) {
  printf("--- Starting pstest ---\n");

  int total_procs = ps_listinfo(0, 0);
  assert_test(total_procs > 0, "ps_listinfo(NULL) returned > 0");

  struct procinfo small_buf[1];
  int req_size = ps_listinfo(small_buf, 1);
  assert_test(req_size > 1 && req_size == total_procs, "Insufficient buffer returns required size > lim");

  int bad_res = ps_listinfo((struct procinfo *)0xffffffffffffffff, 10);
  assert_test(bad_res < 0, "Bad memory address returns negative error code");

  struct procinfo *full_buf = malloc(total_procs * sizeof(struct procinfo));
  int final_count = ps_listinfo(full_buf, total_procs);
  assert_test(final_count == total_procs, "Correctly filled buffer returns exact count");
  
  int my_pid = getpid();
  int found = 0;
  for(int i = 0; i < final_count; i++) {
    if(full_buf[i].pid == my_pid) {
      found = 1;
      break;
    }
  }
  assert_test(found == 1, "Current process found in the returned list");

  free(full_buf);
  printf("--- All tests passed! ---\n");
  exit(0);
}