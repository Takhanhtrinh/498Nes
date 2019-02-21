#include "helper.h"
void print_binary(u8 value) {
  for (int i = 0; i < 8;i++) {
    u8 val = value & 0x80; 
    if (val == 0)
      printf("0");
    else 
      printf("1");
    value = value << 1;
  }
  printf("\n");
}
