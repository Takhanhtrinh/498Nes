/*
 * cpu.c
 * implementation for cpu
 * by: trinh ta
 */

#include "cpu.h"
// create cpu for global variable cpu
void create_cpu()
{
  cpu.a = cpu.x = cpu.y = 0;
  cpu.pc = 0;
  // defalt flag 
  cpu.p = 0x24;
  cpu.stack = 0xFD;
  cpu.cycles = 0x00000000;
}
void zero_page()
{
  opcode_address = read_mem_b(cpu.pc + 1);
#ifdef DEBUG
  printf(" @%02x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
}
void zero_page_x()
{
  // from 0x00 -> 0xff
  opcode_address = (u16)(read_mem_b(cpu.pc + 1) + cpu.x) & 0xFF;
#ifdef DEBUG
  printf("x = %02x @%02x = %02x\n", cpu.x, opcode_address,
         mem.buffer[opcode_address]);
#endif
}
void zero_page_y()
{
  // from 0x00 -> 0xff
  opcode_address = (read_mem_b(cpu.pc + 1)) + cpu.y & 0xFF;
#ifdef DEBUG
  printf(" @%02x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
}
void absolute()
{
  opcode_address = read_mem_w(cpu.pc + 1);
#ifdef DEBUG
  printf(" @%04x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
}
// push value to the stack, and decrease the address, because stack is from
// 100-1ff
void push(u8 val) { write_mem_b(val, (u16)cpu.stack-- | STACK_OFFSET); }
// pop the stack 
u8 pull() { return read_mem_b((u16)++cpu.stack | STACK_OFFSET); }

u8 absolute_x()
{
  // read address and add cpu.x
  u16 address = read_mem_w(cpu.pc + 1);
  opcode_address = address + cpu.x;
  // get page across
  u8 pageAcross = isPageAcross(address, opcode_address);
  // return 1 if 2 addresses are in difrerent page
#ifdef DEBUG
  printf(" @%04x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
  return pageAcross;
}

u8 absolute_y()
{
  // read address and add to cpu.x
  u16 address = read_mem_w(cpu.pc + 1);
  opcode_address = address + cpu.y;
  // get page across
#ifdef DEBUG
  printf(" @%04x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
  return isPageAcross(address, opcode_address);
}
void indirect()
{
  // address from instruction
  u16 val = read_mem_w(cpu.pc + 1);
  // BUG From 6502
  // read address bug if the lower 8 bits are 0xFF. it will fetch the lower byte
  // from xxff and higher byte at ff00
  if ((val & 0x00FF) == 0x00FF)
    opcode_address =
        (u16)read_mem_b(val) | (u16)(read_mem_b(val & 0xFF00) << 8);
  else
    opcode_address = read_mem_w(val);
#ifdef DEBUG
  printf(" @%04x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
}

void indirect_x()
{
  // address from instruction
  u8 val = read_mem_b(cpu.pc + 1) + cpu.x;
  // new read address
  // read address bug if the lower 8 bits are 0xFF. it will fetch the lower byte
  // from xxff and higher byte at ff00
  if ((val & 0x00FF) == 0x00FF)
    opcode_address =
        (u16)read_mem_b(val) | (u16)(read_mem_b(val & 0xFF00) << 8);
  else
    opcode_address = read_mem_w(val);
#ifdef DEBUG
  printf(" @%04x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
}
u8 indirect_y()
{
  // address from instruction
  u8 val = read_mem_b(cpu.pc + 1);
  // new read address
  // read address bug if the lower 8 bits are 0xFF. it will fetch the lower byte
  // from xxff and higher byte at ff00
  if ((val & 0x00FF) == 0x00FF)
    opcode_address =
        (u16)read_mem_b(val) | (u16)(read_mem_b(val & 0xFF00) << 8);
  else
    opcode_address = read_mem_w(val);
  opcode_address += cpu.y;
#ifdef DEBUG
  printf(" @%04x = %02x\n", opcode_address, mem.buffer[opcode_address]);
#endif
  return isPageAcross(opcode_address - cpu.y, opcode_address);
}

void relative()
{
  u8 val = read_mem_b(cpu.pc + 1);
  opcode_address = cpu.pc + 2 + (char)val;
}

int emulate()
{
  // when the ppu is in vblank and the nmi interrupt is on in the 0x2000
  // register
  if (cpu.nmi_interrupt != 0) {
    cpu.nmi_interrupt = 0;
    interrupt_nmi();
    cpu.cycles += 7;
    return 7;
  }
  // next opcode
  u8 opcode = mem.buffer[cpu.pc];
  // use for branching function like jsr, jump, rts, rti, because at the end of
  // the emulate function, we will add opcode size (number of bytes for each
  // opcode). If the opcode is a branch, then we don't need to add the number of
  // bytes for next opcode, because we set a new program counter
  u8 isBranch = 0;
  // disassembler
#ifdef DEBUG
  printf("%04x%5s", cpu.pc, "");
  if (opcodes_size[(u8)opcode] == 1) {
    printf("%s", opcode_name[(u8)opcode]);
  }
  else if (opcodes_size[(u8)opcode] == 2) {
    printf(opcode_name[(u8)opcode], mem.buffer[cpu.pc + 1]);
  }
  else if (opcodes_size[(u8)opcode] == 3) {
    printf(opcode_name[(u8)opcode], mem.buffer[cpu.pc + 2],
           mem.buffer[cpu.pc + 1]);
  }
  else
    printf("cant find it \n");
  printf("%-*s", 25, "");
  // showRegisters();
  printf("cycles: %u ppuCycles: %u scanlines: %u", cpu.cycles, ppu.cycles,
         ppu.scanlines);
  printf("\n");
#endif
  // number of cycles for each instruction for ppu emulate
  u8 numberOfCycles = 0;
  switch (opcode) {
    // ADC
    case 0x69:
      address_mode = IMMEDIATE; // will move to immediate()
      immediate();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 2;
      break;
    case 0x65:
      address_mode = ZERO_PAGE; // will move to zeropage
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 3;
      break;
    case 0x75:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 4;
      break;
    case 0x6D:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 4;
      break;
    case 0x7D:
      address_mode = ABSOLUTE_X;
      // add 1 if the page is a cross
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 4;
      break;
    case 0x79:
      address_mode = ABSOLUTE_Y;
      // add 1 if the page is across;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 4;
      break;
    case 0x61:
      address_mode = INDIRECT_X;
      indirect_x();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 6;
      break;
    case 0x71:
      address_mode = INDIRECT_Y;
      numberOfCycles += indirect_y();
      opcode_value = read_mem_b(opcode_address);
      ADC();
      numberOfCycles += 5;
      break;
    // and
    case 0x29:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 2;
      break;
    case 0x25:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 3;
      break;
    case 0x35:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 4;
      break;
    case 0x2D:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 4;
      break;
    case 0x3D:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 4;
      break;
    case 0x39:
      address_mode = ABSOLUTE_Y;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 4;
      break;
    case 0x21:
      address_mode = INDIRECT_X;
      indirect_x();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 6;
      break;
    case 0x31:
      address_mode = INDIRECT_Y;
      numberOfCycles += indirect_y();
      opcode_value = read_mem_b(opcode_address);
      AND();
      numberOfCycles += 5;
      break;
    case 0x0A:
      address_mode = ACCUMULATOR;
      ASL();
      numberOfCycles += 2;
      break;
    case 0x06:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      ASL();
      numberOfCycles += 5;
      break;
    case 0x16:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      ASL();
      numberOfCycles += 6;
      break;
    case 0x0E:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      ASL();
      numberOfCycles += 6;
      break;
    case 0x1E:
      address_mode = ABSOLUTE_X;
      absolute_x();
      opcode_value = read_mem_b(opcode_address);
      ASL();
      numberOfCycles += 7;
      break;
    case 0x90:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BCC();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0xB0:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BCS();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0xF0:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BEQ();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0x24:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      BIT();
      numberOfCycles += 3;
      break;
    case 0x2C:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      BIT();
      numberOfCycles += 4;
      break;
    case 0x30:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BMI();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0xD0:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BNE();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0x10:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BPL();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0x00:
      address_mode = IMPLIED;
      BRK();
      isBranch = 1;
      numberOfCycles += 7;
      break;
    case 0x50:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BVC();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0x70:
      address_mode = RELATIVE;
      relative();
      numberOfCycles += BVS();
      if (numberOfCycles != 0) isBranch = 1;
      numberOfCycles += 2;
      break;
    case 0x18:
      address_mode = IMPLIED;
      CLC();
      numberOfCycles += 2;
      break;
    case 0xD8:
      address_mode = IMPLIED;
      CLD();
      numberOfCycles += 2;
      break;
    case 0x58:
      address_mode = IMPLIED;
      CLI();
      numberOfCycles += 2;
      break;
    case 0xB8:
      address_mode = IMPLIED;
      CLV();
      numberOfCycles += 2;
      break;
    case 0xC9:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 2;
      break;
    case 0xC5:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 3;
      break;
    case 0xD5:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 4;
      break;
    case 0xCD:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 4;
      break;
    case 0xDD:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 4;
      break;
    case 0xD9:
      address_mode = ABSOLUTE_Y;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 4;
      break;
    case 0xC1:
      address_mode = INDIRECT_X;
      indirect_x();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 6;
      break;
    case 0xD1:
      address_mode = INDIRECT_Y;
      numberOfCycles += indirect_y();
      opcode_value = read_mem_b(opcode_address);
      CMP();
      numberOfCycles += 5;
      break;
    case 0xE0:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      CPX();
      numberOfCycles += 2;
      break;
    case 0xE4:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      CPX();
      numberOfCycles += 3;
      break;
    case 0xEC:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      CPX();
      numberOfCycles += 4;
      break;
    case 0xC0:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      CPY();
      numberOfCycles += 2;
      break;
    case 0xC4:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      CPY();
      numberOfCycles += 3;
      break;
    case 0xCC:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      CPY();
      numberOfCycles += 4;
      break;
    case 0xC6:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      DEC();
      numberOfCycles += 5;
      break;
    case 0xD6:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      DEC();
      numberOfCycles += 6;
      break;
    case 0xCE:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      DEC();
      numberOfCycles += 6;
      break;
    case 0xDE:
      address_mode = ABSOLUTE_X;
      absolute_x();
      opcode_value = read_mem_b(opcode_address);
      DEC();
      numberOfCycles += 7;
      break;
    case 0xCA:
      address_mode = IMPLIED;
      DEX();
      numberOfCycles += 2;
      break;
    case 0x88:
      address_mode = IMPLIED;
      DEY();
      numberOfCycles += 2;
      break;
    case 0x49:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 2;
      break;
    case 0x45:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 3;
      break;
    case 0x55:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 4;
      break;
    case 0x4D:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 4;
      break;
    case 0x5D:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 4;
      break;
    case 0x59:
      address_mode = ABSOLUTE_Y;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 4;
      break;
    case 0x41:
      address_mode = INDIRECT_X;
      indirect_x();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 6;
      break;
    case 0x51:
      address_mode = INDIRECT_Y;
      numberOfCycles += indirect_y();
      opcode_value = read_mem_b(opcode_address);
      EOR();
      numberOfCycles += 5;
      break;
    case 0xE6:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      INC();
      numberOfCycles += 5;
      break;
    case 0xF6:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      INC();
      numberOfCycles += 6;
      break;
    case 0xEE:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      INC();
      numberOfCycles += 6;
      break;
    case 0xFE:
      address_mode = ABSOLUTE_X;
      absolute_x();
      opcode_value = read_mem_b(opcode_address);
      INC();
      numberOfCycles += 7;
      break;
    case 0xE8:
      address_mode = IMPLIED;
      INX();
      numberOfCycles += 2;
      break;
    case 0xC8:
      address_mode = IMPLIED;
      INY();
      numberOfCycles += 2;
      break;
    case 0x4C:
      address_mode = ABSOLUTE;
      absolute();
      JMP();
      isBranch = 1;
      numberOfCycles += 3;
      break;
    case 0x6C:
      address_mode = INDIRECT;
      indirect();
      JMP();
      isBranch = 1;
      numberOfCycles += 5;
      break;
    case 0x20:
      address_mode = ABSOLUTE;
      absolute();
      JSR();
      isBranch = 1;
      numberOfCycles += 6;
      break;
    case 0xA9:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 2;
      break;
    case 0xA5:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 3;
      break;
    case 0xB5:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 4;
      break;
    case 0xAD:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 4;
      break;
    case 0xBD:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 4;
      break;
    case 0xB9:
      address_mode = ABSOLUTE_Y;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 4;
      break;
    case 0xA1:
      address_mode = INDIRECT_X;
      indirect_x();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 6;
      break;
    case 0xB1:
      address_mode = INDIRECT_Y;
      numberOfCycles += indirect_y();
      opcode_value = read_mem_b(opcode_address);
      LDA();
      numberOfCycles += 5;
      break;
    case 0xA2:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      LDX();
      numberOfCycles += 2;
      break;
    case 0xA6:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      LDX();
      numberOfCycles += 3;
      break;
    case 0xB6:
      address_mode = ZERO_PAGE_Y;
      zero_page_y();
      opcode_value = read_mem_b(opcode_address);
      LDX();
      numberOfCycles += 4;
      break;
    case 0xAE:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      LDX();
      numberOfCycles += 4;
      break;
    case 0xBE:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      LDX();
      numberOfCycles += 4;
      break;
    case 0xA0:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      LDY();
      numberOfCycles += 2;
      break;
    case 0xA4:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      LDY();
      numberOfCycles += 3;
      break;
    case 0xB4:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      LDY();
      numberOfCycles += 4;
      break;
    case 0xAC:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      LDY();
      numberOfCycles += 4;
      break;
    case 0xBC:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      LDY();
      numberOfCycles += 4;
      break;
    case 0x4A:
      address_mode = ACCUMULATOR;
      LSR();
      numberOfCycles += 2;
      break;
    case 0x46:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      LSR();
      numberOfCycles += 5;
      break;
    case 0x56:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      LSR();
      numberOfCycles += 6;
      break;
    case 0x4E:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      LSR();
      numberOfCycles += 6;
      break;
    case 0x5E:
      address_mode = ABSOLUTE_X;
      absolute_x();
      opcode_value = read_mem_b(opcode_address);
      LSR();
      numberOfCycles += 7;
      break;
      // NOP
    case 0xEA:
      address_mode = IMPLIED;
      numberOfCycles += 2;
      break;
    case 0x09:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 2;
      break;
    case 0x05:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 3;
      break;
    case 0x15:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 4;
      break;
    case 0x0D:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 4;
      break;
    case 0x1D:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 4;
      break;
    case 0x19:
      address_mode = ABSOLUTE_Y;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 4;
      break;
    case 0x01:
      address_mode = INDIRECT_X;
      indirect_x();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 6;
      break;
    case 0x11:
      address_mode = INDIRECT_Y;
      indirect_y();
      opcode_value = read_mem_b(opcode_address);
      ORA();
      numberOfCycles += 5;
      break;
    case 0x48:
      address_mode = IMPLIED;
      PHA();
      numberOfCycles += 3;
      break;
    case 0x08:
      address_mode = IMPLIED;
      PHP();
      numberOfCycles += 3;
      break;
    case 0x68:
      address_mode = IMPLIED;
      PLA();
      numberOfCycles += 4;
      break;
    case 0x28:
      address_mode = IMPLIED;
      PLP();
      numberOfCycles += 4;
      break;
    case 0x2A:
      address_mode = ACCUMULATOR;
      ROL();
      numberOfCycles += 2;
      break;
    case 0x26:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      ROL();
      numberOfCycles += 5;
      break;
    case 0x36:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      ROL();
      numberOfCycles += 6;
      break;
    case 0x2E:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      ROL();
      numberOfCycles += 6;
      break;
    case 0x3E:
      address_mode = ABSOLUTE_X;
      absolute_x();
      opcode_value = read_mem_b(opcode_address);
      ROL();
      numberOfCycles += 7;
      break;
    case 0x6A:
      address_mode = ACCUMULATOR;
      ROR();
      numberOfCycles += 2;
      break;
    case 0x66:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      ROR();
      numberOfCycles += 5;
      break;
    case 0x76:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      ROR();
      numberOfCycles += 6;
      break;
    case 0x6E:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      ROR();
      numberOfCycles += 6;
      break;
    case 0x7E:
      address_mode = ABSOLUTE_X;
      absolute_x();
      opcode_value = read_mem_b(opcode_address);
      ROR();
      numberOfCycles += 7;
      break;
    case 0x40:
      address_mode = IMPLIED;
      RTI();
      isBranch = 1;
      numberOfCycles += 6;
      break;
    case 0x60:
      address_mode = IMPLIED;
      RTS();
      isBranch = 1;
      numberOfCycles += 6;
      break;
    case 0xE9:
      address_mode = IMMEDIATE;
      immediate();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 2;
      break;
    case 0xE5:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 3;
      break;
    case 0xF5:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 4;
      break;
    case 0xED:
      address_mode = ABSOLUTE;
      absolute();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 4;
      break;
    case 0xFD:
      address_mode = ABSOLUTE_X;
      numberOfCycles += absolute_x();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 4;
      break;
    case 0xF9:
      address_mode = ABSOLUTE_Y;
      numberOfCycles += absolute_y();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 4;
      break;
    case 0xE1:
      address_mode = INDIRECT_X;
      indirect_x();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 6;
      break;
    case 0xF1:
      address_mode = INDIRECT_Y;
      numberOfCycles += indirect_y();
      opcode_value = read_mem_b(opcode_address);
      SBC();
      numberOfCycles += 5;
      break;
    case 0x38:
      address_mode = IMPLIED;
      SEC();
      numberOfCycles += 2;
      break;
    case 0xF8:
      address_mode = IMPLIED;
      SED();
      numberOfCycles += 2;
      break;
    case 0x78:
      address_mode = IMPLIED;
      SEI();
      numberOfCycles += 2;
      break;
    case 0x85:
      address_mode = ZERO_PAGE;
      zero_page();
      opcode_value = read_mem_b(opcode_address);
      STA();
      numberOfCycles += 3;
      break;
    case 0x95:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      opcode_value = read_mem_b(opcode_address);
      STA();
      numberOfCycles += 4;
      break;
    case 0x8D:
      address_mode = ABSOLUTE;
      absolute();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STA();
      numberOfCycles += 4;
      break;
    case 0x9D:
      address_mode = ABSOLUTE_X;
      absolute_x();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STA();
      numberOfCycles += 5;
      break;
    case 0x99:
      address_mode = ABSOLUTE_Y;
      absolute_y();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STA();
      numberOfCycles += 5;
      break;
    case 0x81:
      address_mode = INDIRECT_X;
      indirect_x();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STA();
      numberOfCycles += 6;
      break;
    case 0x91:
      address_mode = INDIRECT_Y;
      indirect_y();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STA();
      numberOfCycles += 6;
      break;
    case 0x86:
      address_mode = ZERO_PAGE;
      zero_page();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STX();
      numberOfCycles += 3;
      break;
    case 0x96:
      address_mode = ZERO_PAGE_Y;
      zero_page_y();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STX();
      numberOfCycles += 4;
      break;
    case 0x8E:
      address_mode = ABSOLUTE;
      absolute();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STX();
      numberOfCycles += 4;
      break;
    case 0x84:
      address_mode = ZERO_PAGE;
      zero_page();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STY();
      numberOfCycles += 3;
      break;
    case 0x94:
      address_mode = ZERO_PAGE_X;
      zero_page_x();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STY();
      numberOfCycles += 4;
      break;
    case 0x8C:
      address_mode = ABSOLUTE;
      absolute();
      if (opcode_address != PPU_R_DATA_ADDRESS)
        opcode_value = read_mem_b(opcode_address);
      else 
        opcode_value = mem.buffer[opcode_address];
      STY();
      numberOfCycles += 4;
      break;
    case 0xAA:
      address_mode = IMPLIED;
      TAX();
      numberOfCycles += 2;
      break;
    case 0xA8:
      address_mode = IMPLIED;
      TAY();
      numberOfCycles += 2;
      break;
    case 0xBA:
      address_mode = IMPLIED;
      TSX();
      numberOfCycles += 2;
      break;
    case 0x8A:
      address_mode = IMPLIED;
      TXA();
      numberOfCycles += 2;
      break;
    case 0x9A:
      address_mode = IMPLIED;
      TXS();
      numberOfCycles += 2;
      break;
    case 0x98:
      address_mode = IMPLIED;
      TYA();
      numberOfCycles += 2;
      break;
    default:
      printf("the opcode doesn't support\n");
      cpu.cycles += numberOfCycles;
      return -1; // return -1 when the opcode doesn't support(unofficial opcode)
  }
  if (!isBranch) cpu.pc += opcodes_size[opcode];
  cpu.cycles += numberOfCycles; // master clock
  return numberOfCycles;
}

void interrupt_nmi()
{
  const u16 NMI_VECTOR = 0xFFFA;
  // push high byte of program counter to stack
  push((cpu.pc & 0xFF00) >> 8);
  // push low byte of program counter to stack
  push(cpu.pc & 0x00FF);
  // push p flag to stack with break flag is clear
  push(cpu.p | 0x20);
  // read new program counter
  cpu.pc = read_mem_w(NMI_VECTOR);
  // turn on interrupt flag
  cpu.p = cpu.p | INTERRUPT_FLAG;
}

void interrupt_reset()
{
  const u16 RESET_VECTOR = 0xFFFC;
  // set new program counter
  cpu.pc = read_mem_w(RESET_VECTOR);
  // set flag
  cpu.p = 0x24;
  // reset stack pointer
  cpu.stack = 0xFD;
}

void interrupt_irq()
{
  const u16 IRQ_VECTOR = 0xFFFE;
  // push high byte of program counter to stack
  push((cpu.pc & 0xFF00) >> 8);
  // push low byte of program counter to stack
  push(cpu.pc & 0x00FF);
  // push p flag to stack with b Flag is clear
  push(cpu.p | 0x20);
  // read new program counter
  cpu.pc = read_mem_w(IRQ_VECTOR);
  // turn on interrupt flag
  cpu.p = cpu.p | INTERRUPT_FLAG;
}

void setNZ(u8 value)
{
  setN(value);
  setZ(value);
}
// the order of opcode implementation base on this website
// http://www.obelisk.me.uk/6502/reference.html
void ADC()
{
  u8 isCarry = (getCFlag() != 0) ? 1 : 0;
  // use to check carry flag
  u8 a = cpu.a;
  cpu.a += opcode_value + isCarry;
  setNZ(cpu.a);
  // if the cpu.a 7th bit is off after adding, but before it is on, that means
  // the carry flag is on
  if (((u16)a + (u16)isCarry + (u16)opcode_value) >= 0x100)
    SEC();
  else
    CLC();
  // check the overflow flag, if 2 positive number are adding together but the
  // result is negative number then the overflow flag is on

  // check 2 number are same sign or not (before adding)
  u8 isSameSign = (((a ^ opcode_value) & 0x80) != 0x80) ? 1 : 0;
  // check 2 numbers are different sign (after adding)
  u8 isDiffSign = (((cpu.a ^ a) & 0x80) == 0x80) ? 1 : 0;

  if (isSameSign && isDiffSign)
    cpu.p = cpu.p | OVERFLOW_FLAG;
  else
    cpu.p = cpu.p & ~(OVERFLOW_FLAG);
}

void AND()
{
  // a and with the value from memory
  cpu.a &= opcode_value;
  setNZ(cpu.a);
}

void ASL()
{
  if (address_mode == ACCUMULATOR) {
    // set carry flag if the last bit is on, otherwise clear it
    cpu.p = ((cpu.a & 0x80) == 0x80) ? (cpu.p | CARRY_FLAG)
                                     : (cpu.p & ~(CARRY_FLAG));
    // shift left 1 bit
    cpu.a <<= 1;
    setNZ(cpu.a);
  }
  //
  else {
    cpu.p = ((opcode_value & 0x80) == 0x80) ? (cpu.p | CARRY_FLAG)
                                            : (cpu.p & ~(CARRY_FLAG));
    // shift left 1 bit
    opcode_value <<= 1;
    write_mem_b(opcode_value, opcode_address);
    setNZ(opcode_value);
  }
}

u8 BCC()
{
  // if carry flag is clear than jump to the addres
  if (getCFlag() == 0) {
    cpu.pc = opcode_address;
    // return 1 when branching is success for clock
    return 1;
  }
  else
    return 0;
}
u8 BCS()
{
  // if carry flag is clear than jump to the addres
  if (getCFlag() != 0) {
    cpu.pc = opcode_address;
    // return 1 when branching is success for clock
    return 1;
  }
  else
    return 0;
}
u8 BEQ()
{
  // if zero flag is set
  if (getZFlag() != 0) {
    cpu.pc = opcode_address;
    // return 1 when branching is success for clock
    return 1;
  }
  else
    return 0;
}

void BIT()
{
  u8 temp = cpu.a & opcode_value;
  setZ(temp);
  // set overflow flag if the value's bit 6 is on, otherwise clear it
  cpu.p = (((opcode_value >> 6) & 0x01) == 0x01)
              ? (cpu.p = cpu.p | OVERFLOW_FLAG)
              : (cpu.p = cpu.p & ~(OVERFLOW_FLAG));
  setN(opcode_value);
}

u8 BMI()
{
  if (getNFlag() != 0) {
    cpu.pc = opcode_address;
    // return 1 when branching is success for clock
    return 1;
  }
  return 0;
}
u8 BNE()
{
  if (getZFlag() == 0) {
    cpu.pc = opcode_address;
    // return 1 when branching is success for clock
    return 1;
  }
  return 0;
}
u8 BPL()
{
  if (getNFlag() == 0) {
    cpu.pc = opcode_address;
    // return 1 when branching is success for clock
    return 1;
  }
  return 0;
}

void BRK()
{
  // push high byte
  push((cpu.pc & 0xFF00) >> 8);
  // push low byte
  push(cpu.pc & 0x00FF);
  // push flag https://wiki.nesdev.com/w/index.php/Status_flags
  push(cpu.p | 0x30);
  const u16 BRK_VECTOR = 0xFFFE;
  cpu.pc = read_mem_w(BRK_VECTOR);
  cpu.p = cpu.p | INTERRUPT_FLAG;
}
u8 BVC()
{
  if (getVFlag() == 0) {
    cpu.pc = opcode_address;
    return 1;
  }
  return 0;
}
u8 BVS()
{
  if (getVFlag() != 0) {
    cpu.pc = opcode_address;
    return 1;
  }
  return 0;
}
void CMP()
{
  // value a >= opcode_value then we set the carry bit on
  if (cpu.a >= opcode_value)
    SEC();
  else
    CLC();
  setNZ(cpu.a - opcode_value);
}
void CPX()
{
  if (cpu.x >= opcode_value)
    SEC();
  else
    CLC();
  setNZ(cpu.x - opcode_value);
}
void CPY()
{
  if (cpu.y >= opcode_value)
    SEC();
  else
    CLC();
  setNZ(cpu.y - opcode_value);
}
void DEC()
{
  opcode_value--;
  setNZ(opcode_value);
  write_mem_b(opcode_value, opcode_address);
}
void DEX()
{
  cpu.x--;
  setNZ(cpu.x);
}

void DEY()
{
  cpu.y--;
  setNZ(cpu.y);
}

void EOR()
{
  // xor
  cpu.a = cpu.a ^ opcode_value;
  setNZ(cpu.a);
}
void INC()
{
  opcode_value++;
  write_mem_b(opcode_value, opcode_address);
  setNZ(opcode_value);
}

void INX()
{
  cpu.x++;
  setNZ(cpu.x);
}
void INY()
{
  cpu.y++;
  setNZ(cpu.y);
}

void JSR()
{
  // push high byte
  // + 2 because jsr is 3 bytes - 1 byte (according to 6502 doc)
  push((cpu.pc + 2) >> 8);
  // push low byte
  push((cpu.pc + 2) & 0x00FF);
  cpu.pc = opcode_address;
}

void LDA()
{
  // load a from mem
  cpu.a = opcode_value;
  setNZ(cpu.a);
}
void LDX()
{
  // load x from mem
  cpu.x = opcode_value;
  setNZ(cpu.x);
}

void LDY()
{
  // load y from mem
  cpu.y = opcode_value;
  setNZ(cpu.y);
}

  // shift right
void LSR()
{
  if (address_mode == ACCUMULATOR) {
    // set to contents of old bit 0
    if ((cpu.a & 0x01) == 0x01)
      cpu.p = cpu.p | CARRY_FLAG;
    else
      cpu.p = cpu.p & ~(CARRY_FLAG);
    cpu.a >>= 1;
    setNZ(cpu.a);
  }
  else {
    // set to contents of old bit 0
    if ((opcode_value & 0x01) == 0x01)
      cpu.p = cpu.p | CARRY_FLAG;
    else
      cpu.p = cpu.p & ~(CARRY_FLAG);
    opcode_value >>= 1;
    write_mem_b(opcode_value, opcode_address);
    setNZ(opcode_value);
  }
}
void ORA()
{
  // or bitwise operator
  cpu.a = cpu.a | opcode_value;
  setNZ(cpu.a);
}
void PLA()
{
  // pull value from stack and set to a
  cpu.a = pull();
  setNZ(cpu.a);
}
void ROL()
{
  if (address_mode == ACCUMULATOR) {
    u8 a = cpu.a;
    if ((a >> 7) == 0x01)
      cpu.p = cpu.p | CARRY_FLAG;
    else
      cpu.p = cpu.p & ~(CARRY_FLAG);
    cpu.a <<= 1;
    // turn zero bit on if the old content of 7th is on
    cpu.a = cpu.a | getCFlag();
    setNZ(cpu.a);
  }
  else {
    u8 m = opcode_value;
    if ((m >> 7) == 0x01)
      cpu.p = cpu.p | CARRY_FLAG;
    else
      cpu.p = cpu.p & ~(CARRY_FLAG);
    opcode_value <<= 1;
    // turn zero bit on if the old content of 7th is on
    opcode_value = opcode_value | getCFlag();
    setNZ(opcode_value);
    write_mem_b(opcode_value, opcode_address);
  }
}
void ROR()
{
  if (address_mode == ACCUMULATOR) {
    // copy old value of a
    u8 a = cpu.a;
    // shift right
    cpu.a >>= 1;
    // turn 7th bit on if the carry bit is on
    cpu.a = cpu.a | ((getCFlag() << 7) & 0x80);
    // set negative and zero flag
    // set carry flag if 0 bit is on, otherwise clear it
    setNZ(cpu.a);
    if ((a & 0x01) == 0x01)
      cpu.p = cpu.p | CARRY_FLAG;
    else
      cpu.p = cpu.p & ~(CARRY_FLAG);
  }
  else {
    // copy old value of m
    u8 m = opcode_value;
    // shift right
    opcode_value >>= 1;
    // turn 7th bit on if the carry  bit  is on
    opcode_value = opcode_value | (getCFlag() << 7);
    // set negative and zero flag
    // set carry flag if 0 bit is on, otherwise clear it
    setNZ(opcode_value);
    if ((m & 0x01) == 0x01)
      cpu.p = cpu.p | CARRY_FLAG;
    else
      cpu.p = cpu.p & ~(CARRY_FLAG);
    write_mem_b(opcode_value, opcode_address);
  }
}

void RTI()
{
  // pull flag register
  PLP();
  // fetch lowbyte
  u8 lowByte = pull();
  // fetch high byte
  u8 highByte = pull();
  // new program counter
  cpu.pc = ((u16)(highByte) << 8) | (u16)lowByte;
}
void RTS()
{
  // fetch lowByte
  u8 lowByte = pull();
  // fetch high byte
  u8 hightByte = pull();
  // new program counter
  cpu.pc = ((u16)(hightByte) << 8) | (u16)lowByte;
  cpu.pc += 1;
}
void SBC()
{
  // substract form carry or (a-b = a + (-b))
  opcode_value = ~opcode_value;
  ADC();
}
void STA() { write_mem_b(cpu.a, opcode_address); }
void STX() { write_mem_b(cpu.x, opcode_address); }
void STY() { write_mem_b(cpu.y, opcode_address); }
void TSX()
{
  cpu.x = cpu.stack;
  setNZ(cpu.x);
}
void TXA()
{
  cpu.a = cpu.x;
  setNZ(cpu.a);
}
