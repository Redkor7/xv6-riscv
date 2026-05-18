#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include <stdarg.h>

uint log_mask = 0; 
uint log_end_ticks = 0;

struct {
  struct spinlock lock;
  char buf[DMESG_SIZE];
  uint head;
  uint tail;
} dmesg;

void 
dmesginit(void) {
  initlock(&dmesg.lock, "dmesg");
  dmesg.buf[0] = '\n';
  dmesg.head = 1;
  dmesg.tail = 0;
}

void 
dmesg_putc(char c) {
  dmesg.buf[dmesg.head] = c;
  dmesg.head = (dmesg.head + 1) % DMESG_SIZE;
  
  if(dmesg.head == dmesg.tail) {
    dmesg.tail = (dmesg.tail + 1) % DMESG_SIZE;
  }
}

static void 
dmesg_printint(int xx, int base, int sign) {
  static char d[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = d[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    dmesg_putc(buf[i]);
}

void 
pr_msg(const char *fmt, ...) {
  va_list ap;
  int i, c;
  char *s;

  acquire(&tickslock);
  uint t = ticks;
  release(&tickslock);

  acquire(&dmesg.lock);

  dmesg_putc('[');
  dmesg_printint(t, 10, 0);
  dmesg_putc(']');
  dmesg_putc(' ');

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      dmesg_putc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0) break;
    switch(c){
    case 'd':
      dmesg_printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      dmesg_printint(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      dmesg_printint(va_arg(ap, uint64), 16, 0);
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        dmesg_putc(*s);
      break;
    case '%':
      dmesg_putc('%');
      break;
    default:
      dmesg_putc('%');
      dmesg_putc(c);
      break;
    }
  }
  va_end(ap);

  dmesg_putc('\n');
  release(&dmesg.lock);
}

int should_log(uint type) {
  if (!(log_mask & type)) return 0;
  if (log_end_ticks > 0) {
    acquire(&tickslock);
    uint t = ticks;
    release(&tickslock);
    if (t > log_end_ticks) {
      log_mask = 0; 
      return 0;
    }
  }
  return 1;
}

int dmesg_r(uint64 user_buf, int max_len) {
  acquire(&dmesg.lock);
  int curr = dmesg.tail;
  
  if (curr != 0) {
      while(curr != dmesg.head && dmesg.buf[curr] != '\n') {
          curr = (curr + 1) % DMESG_SIZE;
      }
      if(curr != dmesg.head) curr = (curr + 1) % DMESG_SIZE;
  } else {
      curr = (curr + 1) % DMESG_SIZE;
  }

  int copied = 0;
  while(curr != dmesg.head && copied < max_len - 1) {
    char c = dmesg.buf[curr];
    if(copyout(myproc()->pagetable, user_buf + copied, &c, 1) < 0) break;
    copied++;
    curr = (curr + 1) % DMESG_SIZE;
  }
  release(&dmesg.lock);

  char null_byte = '\0';
  copyout(myproc()->pagetable, user_buf + copied, &null_byte, 1);

  return copied;
}

void dmesg_set(uint mask, int duration) {
  log_mask = mask;
  if(duration > 0) {
    acquire(&tickslock);
    log_end_ticks = ticks + duration;
    release(&tickslock);
  } else {
    log_end_ticks = 0;
  }
}