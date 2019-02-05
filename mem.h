#ifndef MEM_H
#define MEM_H
#include <stdio.h>
#include <stdlib.h>
#include "definition.h"
#include "ppu.h"
#include "controller.h"
#define MEM_SIZE 0x10000

typedef struct {
  // memory
  u8 buffer[MEM_SIZE];
} MEM;
extern MEM mem;
MEM create_mem();
void write_mem_w(u16 value, u16 address);
u16 read_mem_w(u16 address);
void write_mem_b(u8 value, u16 address);
u8 read_mem_b(u16 address);
void getMirrorAddress(u16 * address);
#endif
