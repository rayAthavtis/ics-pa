#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  int val;
  char expr[32];
  bool is_on;

} WP;

#endif

WP *new_wp(char *, int);
void free_wp(int);
bool check_wp();
void print_wp();
