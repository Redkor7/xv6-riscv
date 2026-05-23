#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

static struct spinlock rtc_lock;

void
rtc_init(void)
{
  initlock(&rtc_lock, "goldfish_rtc");
}

uint64
rtc_get_time(void)
{
  uint32 low, high;

  acquire(&rtc_lock);
  
  low = *(volatile uint32*)(RTC_TIME_LOW);
  high = *(volatile uint32*)(RTC_TIME_HIGH);
  
  release(&rtc_lock);

  return ((uint64)high << 32) | (uint64)low;
}