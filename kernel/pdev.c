#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

enum {
  MINOR_NULL     = 0,
  MINOR_ZERO     = 1,
  MINOR_URANDOM  = 2,
  MINOR_NULLSTAT = 3
};

static struct spinlock rng_lock;
static struct spinlock counter_lock;

static uint64 rng_state = 0xDEADBEEF12345678ULL;
static uint64 dropped_bytes_count = 0;
static const char zeroes[64] = {0};

int pdevread(int minor, int user_dst, uint64 dst, int n) {
  if(n < 0) 
    return -1;

  switch(minor) {
    case MINOR_NULL:
      return 0;

    case MINOR_ZERO: {
      int bytes_read = 0;
      while(bytes_read < n) {
        int chunk = n - bytes_read;
        if(chunk > sizeof(zeroes)) 
          chunk = sizeof(zeroes);
        
        if(either_copyout(user_dst, dst + bytes_read, (void*)zeroes, chunk) < 0)
          return -1;
          
        bytes_read += chunk;
      }
      return n;
    }

    case MINOR_URANDOM: {
      char rand_buf[64];
      int bytes_read = 0;
      while(bytes_read < n) {
        int chunk = n - bytes_read;
        if(chunk > sizeof(rand_buf)) 
          chunk = sizeof(rand_buf);

        acquire(&rng_lock);
        for(int i = 0; i < chunk; i++) {
          rng_state = rng_state * 6364136223846793005ULL + 1442695040888963407ULL;
          rand_buf[i] = (char)(rng_state >> 32); 
        }
        release(&rng_lock);

        if(either_copyout(user_dst, dst + bytes_read, rand_buf, chunk) < 0)
          return -1;
          
        bytes_read += chunk;
      }
      return n;
    }

    case MINOR_NULLSTAT: {
      if(n != sizeof(uint64)) 
        return -1;

      uint64 current_count;
      acquire(&counter_lock);
      current_count = dropped_bytes_count;
      release(&counter_lock);

      if(either_copyout(user_dst, dst, (void*)&current_count, sizeof(current_count)) < 0)
        return -1;
        
      return sizeof(uint64);
    }

    default:
      return -1;
  }
}

int pdevwrite(int minor, int user_src, uint64 src, int n) {
  if(n < 0) return -1;

  switch(minor) {
    case MINOR_NULL:
      return n;

    case MINOR_ZERO:
      return -1;

    case MINOR_URANDOM: {
      if(n != sizeof(uint64)) 
        return -1;

      uint64 new_seed;
      if(either_copyin(&new_seed, user_src, src, sizeof(new_seed)) < 0)
        return -1;

      acquire(&rng_lock);
      rng_state = new_seed;
      release(&rng_lock);
      return n;
    }

    case MINOR_NULLSTAT: {
      acquire(&counter_lock);
      dropped_bytes_count += n;
      release(&counter_lock);
      return n;
    }

    default:
      return -1;
  }
}

void pdevinit(void) {
  initlock(&rng_lock, "pdev_rng");
  initlock(&counter_lock, "pdev_cnt");
  
  devsw[2].read = pdevread;
  devsw[2].write = pdevwrite;
}