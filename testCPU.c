#include <math.h>
#include "cpu.h"
#include "helper.h"
#include "mem.h"
#include "ppu.h"
#include "rom.h"

// global variable for the system
CPU cpu;
// for log
CPU cpu1;
Controller controller;
MEM mem;
u16 opcode_address = 0;
u8 opcode_value = 0;
u8 address_mode = 0;
PPU ppu;
INES ines;
// for reading the log
FILE* testFile;

// print the flag in binary 
void showFlag() { print_binary(cpu.p); }
// print all registers informtion
void showRegisters(CPU* cpu, PPU * ppu) {
  if (cpu == NULL || ppu == NULL ) return;
  printf("pc: %04x ", cpu->pc);
  printf("A:%02x ", cpu->a);
  printf("X:%02x ", cpu->x);
  printf("Y:%02x ", cpu->y);
  printf("P:%02x ", cpu->p);
  printf("SP:%02x ", cpu->stack);
  printf("cycles: %llu ", cpu->cycles);
  printf("ppu cycles: %u ",ppu->cycles);
  printf("scanlines: %u ", ppu->scanlines);
}
// load next line from the log
void loadNesTest() {
  fscanf(testFile, "%hx%hhx%hhx%hhx%hhx%hhx%u\n", &cpu1.pc, &cpu1.a, &cpu1.x,
         &cpu1.y, &cpu1.p, &cpu1.stack, &cpu1.cycles);
}

// show the memory value, for debug
void showMem(u16 from, u16 to) {
  if (from > to) return;
  for (u16 f = from; f <= to; f += 0x10) {
    printf("%04x", f);
    for (int i = 0; i < 0x10; i++) {
      printf("%02x", mem.buffer[f + i]);
    }
    printf("\n");
  }
}
void test() {
  int status = read_room("./donkey.nes");
  if (status != 1) {
    printf("error while reading file\n");
    return;
  }
  // cpu.pc = 0xC000;
  interrupt_reset();
  cpu.cycles = 7;
  // interrupt_reset();
  printf("address: %04x\n", cpu.pc);
}
// compare the value of the emulator and the result from the log 
byte comapre() {
  if (cpu.pc != cpu1.pc || cpu.a != cpu1.a || cpu.x != cpu1.x ||
      cpu.y != cpu1.y || cpu.p != cpu1.p || cpu.stack != cpu1.stack ||
      cpu.cycles != cpu1.cycles)
    return -1;
  return 1;
}
// this function use to test all the opcodes 
void testOpcode() {
  read_room("./nestest.nes");
  cpu.pc = 0xC000;
  cpu.cycles = 7;
  testFile = fopen("./nestes.log", "r");
  byte flag = 1;
  if (testFile == NULL)
    exit(0);
  else {
    loadNesTest();
    while (flag == 1) {
      loadNesTest();
      byte support = 1;
      support = emulate();
      flag = comapre();
      // if comapre the emulator's value and the log value are not the same, and
      // the emulator supprts the opcode. That means something wrong with the
      // mulator
      if (flag == -1 && support >= 0) {
        printf("error\n");
        printf("pc: %04x\n", cpu.pc);
        // show the emulator registers
        showRegisters(&cpu, &ppu);
        printf("\n");
        // show the log regsters
        showRegisters(&cpu1, &ppu);
        printf("\n");
        byte c;
        printf("enter 1 to continue, enter 2 to exit:");
        scanf("%hhu", &c);
        // set the opcode value = to the log for test next opcode, otherwise
        // exit
        if (c == 1) {
          cpu.pc = cpu1.pc;
          cpu.a = cpu1.a;
          cpu.x = cpu1.x;
          cpu.y = cpu1.y;
          cpu.p = cpu1.p;
          cpu.stack = cpu1.stack;
          cpu.cycles = cpu1.cycles;
          flag = 1;
          continue;
        } else {
          printf("terminate\n");
          return;
        }
        // if the opcode doesn't support just need to set the value from the log
        // to the emulator
      } else if (flag == -1 && support == -1) {
        printf("not supported\n");
        showRegisters(&cpu,&ppu);
        printf("\n\n");
        showRegisters(&cpu1, &ppu);
        printf("\n\n");
        cpu.pc = cpu1.pc;
        cpu.a = cpu1.a;
        cpu.x = cpu1.x;
        cpu.y = cpu1.y;
        cpu.p = cpu1.p;
        cpu.stack = cpu1.stack;
        cpu.cycles = cpu1.cycles;
        flag = 1;
      }
    }
  }
}
int main() {
  create_cpu();
  createController();
  create_mem();
  create_ppu();
   testOpcode();
}
