#ifndef _PROCINFO_
#define _PROCINFO_

#define NAME_LEN 16

struct procinfo {
  int pid;
  char name[NAME_LEN];
  int state;
  int ppid;
  char pname[NAME_LEN];
};

#endif