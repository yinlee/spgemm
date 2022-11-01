#ifndef ENTRY_H
#define ENTRY_H

typedef struct entry{
  // int row;
  int length;
  float sub_array[1024];
  int sub_idx[1024];
} Entry;

#endif