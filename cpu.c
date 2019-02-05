#include "cpu.h"
CPU create_cpu() {
  CPU c;
  c.a = c.x = c.y = 0;
  c.pc = 0;
  c.p = 0x24;
  c.stack = 0xFD;
  c.cycles = 0x00000000;
  return c;
}
void setNFlag(u8 value) {
  if ((value & 0x80) == 0x80) 
    setFlag(cpu.p | NEGATIVE_FLAG);
  else 
    setFlag(cpu.p & ~(NEGATIVE_FLAG));
}
void setVFlag(u8 value1, u8 value2, u8 result) {
  u8 flag = (~(value1 ^ value2)) & (value2 ^ result) & 0x80;
  if (flag == 0x80)
    setFlag(cpu.p | OVERFLOW_FLAG);
  else
    setFlag(cpu.p & ~(OVERFLOW_FLAG));
}
void setZFlag(u8 value) {
  if (value == 0)
    setFlag(cpu.p | ZERO_FLAG);
  else
    setFlag(cpu.p & ~(ZERO_FLAG));
}
void setFlag(u8 value) {
  cpu.p = 0;
  cpu.p |= value & 0xff;
  cpu.p |= 0x20;  // this flag always on
}

u8 getNflag() {
  u8 v = cpu.p;
  v = v & NEGATIVE_FLAG;
  return v;
}
u8 getVflag() {
  u8 v = cpu.p;
  v = v & OVERFLOW_FLAG;
  return v;
}
u8 getZflag() {
  u8 v = cpu.p;
  v = v & ZERO_FLAG;
  return v;
}
u8 getCflag() {
  u8 v = cpu.p;
  v = v & CARRY_FLAG;
return v;
}
u8 getIflag() {
  u8 v = cpu.p;
  v = v & INTERRUPT_FLAG;
  return v;
}
u8 getDflag() {
  u8 v = cpu.p;
  v = v & DECIMAL_FLAG;
  return v;
}
u8 getBflag() {
  u8 v = cpu.p;
  v = v & BREAK_FLAG;
  return v;
}

void immediate() {
  address_mode = IMMEDIATE;
  opcode_value = mem.buffer[cpu.pc + 1];
}
void zero_page() {
  address_mode = ZERO_PAGE;
  u8 addrr = mem.buffer[cpu.pc + 1];
  opcode_address = addrr;
  opcode_value = read_mem_b(addrr);
}
void zero_page_x() {
  address_mode = ZERO_PAGE_X;
  u8 addrr = mem.buffer[cpu.pc + 1];
  opcode_address = (u8)(addrr + cpu.x);
  opcode_value = read_mem_b(opcode_address);
  //opcode_value = mem.buffer[opcode_address];
}
void zero_page_y() {
  address_mode = ZERO_PAGE_Y;
  u8 addrr = mem.buffer[cpu.pc + 1];
  opcode_address = (u8)(addrr + cpu.y);
  opcode_value = read_mem_b(opcode_address);
  //opcode_value = mem.buffer[opcode_address];
  //printf("%04x = $%02x\n", opcode_address, opcode_value);
}
void relative() {
  address_mode = RELATIVE;
  //byte value = (byte)(mem.buffer[cpu.pc + 1]);
  //opcode_value = read_mem_b(cpu.pc + 1);
  opcode_value = mem.buffer[cpu.pc + 1];
}
void absolute() {
  address_mode = ABSOLUTE;
  opcode_address = read_mem_w(cpu.pc + 1);
  opcode_value = read_mem_b(opcode_address);

}
u8 absolute_x() {
  address_mode = ABSOLUTE_X;
  u16 val = read_mem_w(cpu.pc + 1);
  u8 rt;
  rt = ((((val & 0x00FF) + (u16)cpu.x) & 0x0F00) != 0) ? 1 : 0;
  opcode_address = val + cpu.x;
  opcode_value = read_mem_b(opcode_address);
  return rt;
}
u8 absolute_y() {
  address_mode = ABSOLUTE_Y;
  u16 val = read_mem_w(cpu.pc + 1);
  u8 rt;
  rt = ((((val & 0x00FF) + (u16)cpu.y) & 0x0F00) != 0) ? 1 : 0;
  opcode_address = val + cpu.y;
  opcode_value = read_mem_b(opcode_address);
  return rt;
}
void indirect() {
  address_mode = INDIRECT;
  opcode_address = read_mem_w(cpu.pc + 1);
  opcode_value = read_mem_b(opcode_address);
}

void index_indirect() {
  address_mode = IN_INDIRECT;
  u8 val = mem.buffer[cpu.pc + 1];
  opcode_address = (u16)(val + cpu.x);
  opcode_address = read_mem_w(opcode_address);
  opcode_value = read_mem_b(opcode_value);
//  if ((opcode_address & 0x00FF) != 0x00FF) {
//    opcode_address = read_mem_w(opcode_address & 0x00FF);
//    opcode_value = read_mem_w(opcode_address);
//  } else {
//    u16 lowByte = read_mem_b(opcode_address & 0x00FF);
//    printf("low byte: %04x\n", lowByte);
//    u16 highByte = read_mem_b(opcode_address & 0x0000) << 8;
//    printf("high byte: %04x\n", highByte);
//    opcode_address = highByte | lowByte;
//    opcode_value = read_mem_w(opcode_address);
//  }
  //printf(" = %04x @ %04x = %02x\n", opcode_address, opcode_address,
         //opcode_value);
}
u8 indirect_indexed() {
  address_mode = IN_INDEXED;
  u8 val = mem.buffer[cpu.pc + 1];
  u8 rt = 0;
  if ((cpu.y & 0x0F) == 0x0f) rt = 1;
  if ((val & 0xFF) == 0xFF) {
    rt = 1;
    u16 lowByte = read_mem_b((u16)val & 0x00FF);
    u16 heightByte = read_mem_b((u16)val & 0x0000) << 8;
    opcode_address = (heightByte | lowByte) + cpu.y;
    opcode_value = read_mem_w(opcode_address);
    return rt;
  } else {
    opcode_address = read_mem_w(val & 0x00FF);
    if ((opcode_address & 0x000F) == 0x000F) rt = 1;
    opcode_address += cpu.y;
    opcode_value = read_mem_w(opcode_address);
    return rt;
  }
}
void implied() {
  address_mode = IMPLIED;
}

void interrupt_reset() {
  const u16 RESET_VECTOR = 0xFFFC;
  u16 pc = read_mem_w(RESET_VECTOR);
  cpu.pc = pc;
  cpu.cycles += 7;
}
void interrupt_irq() {
  // padding byte, ignored by the cpu
  // u8 padding = mem.buffer[cpu.pc + 1];
  const u16 IRQ_VECTOR = 0xFFFE;
  u16 newpc = cpu.pc + 2;
  write_mem_b((newpc >> 8) & 0xFF, cpu.stack-- + STACK_OFFSET);
  write_mem_b((newpc & 0xFF), cpu.stack-- + STACK_OFFSET);
  write_mem_b(cpu.p, cpu.stack-- + STACK_OFFSET);
  cpu.pc = read_mem_w(IRQ_VECTOR);
  setFlag(cpu.p | INTERRUPT_FLAG);
}

// 1    PC     R  fetch opcode (and discard it - $00 (BRK) is forced into the
// opcode register instead) 2    PC     R  read next instruction byte (actually
// the same as above, since PC increment is suppressed. Also discarded.) 3
// $0100,S  W  push PCH on stack, decrement S 4  $0100,S  W  push PCL on stack,
// decrement S
//*** At this point, the signal status determines which interrupt vector is used
//***
// 5  $0100,S  W  push P on stack (with B flag *clear*), decrement S
// 6   A       R  fetch PCL (A = FFFE for IRQ, A = FFFA for NMI), set I flag
// 7   A       R  fetch PCH (A = FFFF for IRQ, A = FFFB for NMI)
void interrupt_nmi() {
  const u16 NMI_VECTOR = 0xFFFA;
  u16 newpc = cpu.pc + 1;
  write_mem_b((newpc >> 8) & 0xFF, cpu.stack-- + STACK_OFFSET);
  write_mem_b((newpc & 0xFF), cpu.stack-- + STACK_OFFSET);
  // turn on b flag, http://wiki.nesdev.com/w/index.php/Status_flags
  write_mem_b(cpu.p | 0x20, cpu.stack-- + STACK_OFFSET);
  cpu.pc = read_mem_w(NMI_VECTOR);
  cpu.p |= INTERRUPT_FLAG;

}
void LDA() {
  cpu.a = opcode_value;
  setNFlag(cpu.a);
  setZFlag(cpu.a);
}
void LDX() {
  cpu.x = opcode_value;
  setNFlag(cpu.x);
  setZFlag(cpu.x);
}
void LDY() {
  cpu.y = opcode_value;
  setNFlag(cpu.y);
  setZFlag(cpu.y);
}
void NOP() {
  return;
}
void STA() {
  // fix the bug when the cpu reads the $2007 to get the value, but STA uses for
  // transfer data to mem
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address -= getAddressIncrement();
  write_mem_b(cpu.a, opcode_address);
  //mem.buffer[opcode_address] = cpu.a;
}
void STX() {
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address-= getAddressIncrement();
  write_mem_b(cpu.x, opcode_address);
}
void STY() {
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address -= getAddressIncrement();
  write_mem_b(cpu.y, opcode_address);
}
void TAX() {
  setNFlag(cpu.a);
  setZFlag(cpu.a);
  cpu.x = cpu.a;
}

void TAY() {
  setNFlag(cpu.a);
  setZFlag(cpu.a);
  cpu.y = cpu.a;
}
void TSX() {
  setNFlag(cpu.stack);
  setZFlag(cpu.stack);
  cpu.x = cpu.stack;
}
void TXA() {
  setNFlag(cpu.x);
  setZFlag(cpu.x);
  cpu.a = cpu.x;
}
void TXS() {
  cpu.stack = cpu.x;
}

void TYA() {
  setNFlag(cpu.y);
  setZFlag(cpu.y);
  cpu.a = cpu.y;
}

void AND() {
  opcode_value &= cpu.a;
  setNFlag(opcode_value);
  setZFlag(opcode_value);
  cpu.a = opcode_value;
}
void ASL() {
  const u8 MASK = 0x80;
  if (address_mode == ACCUMULATOR) {
    if ((cpu.a & MASK) == MASK)
      setFlag(cpu.p | CARRY_FLAG);
    else
      setFlag(cpu.p & ~(CARRY_FLAG));
    cpu.a <<= 1;
    setZFlag(cpu.a);
    setNFlag(cpu.a);
  } else {
    u8 val = (u8)opcode_value;
    if ((val & MASK) == MASK)
      setFlag(cpu.p | CARRY_FLAG);
    else
      setFlag(cpu.p & ~(CARRY_FLAG));
    val <<= 1;
    setZFlag(val);
    setNFlag(val);
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address-= getAddressIncrement();
    write_mem_b(val, opcode_address);
  }
}
// BIT - Bit Test
// A & M, N = M7, V = M6
//
// This instructions is used to test if one or more bits are set in a target
// memory location. The mask pattern in A is ANDed with the value in memory to
// set or clear the zero flag, but the result is not kept. Bits 7 and 6 of the
// value from memory are copied into the N and V flags.
//
// Processor Status after use:
//
// C	Carry Flag	Not affected
// Z	Zero Flag	Set if the result if the AND is zero
// I	Interrupt Disable	Not affected
// D	Decimal Mode Flag	Not affected
// B	Break Command	Not affected
// V	Overflow Flag	Set to bit 6 of the memory value
// N	Negative Flag	Set to bit 7 of the memory value

void BIT() {
  u8 val = cpu.a & opcode_value;
  setZFlag(val);
  setNFlag(opcode_value);
  u8 vMask = 0x40;
  if ((opcode_value & vMask) == vMask)
    setFlag(cpu.p | OVERFLOW_FLAG);  // turn the v flag on
  else
    setFlag(cpu.p & ~(OVERFLOW_FLAG));  // disable v flag
}
void EOR() {
  opcode_value ^= cpu.a;
  setNFlag(opcode_value);
  setZFlag(opcode_value);
  cpu.a = opcode_value;
}
void LSR() {
  const u8 MASK = 0x01;
  if (address_mode == ACCUMULATOR) {
    if ((cpu.a & MASK) == MASK)
      setFlag(cpu.p | CARRY_FLAG);
    else
      setFlag(cpu.p & ~(CARRY_FLAG));
    cpu.a >>= 1;
    setZFlag(cpu.a);
    setNFlag(cpu.a);
  } else {
    u8 val = (u8)opcode_value;
    if ((val & MASK) == MASK)
      setFlag(cpu.p | CARRY_FLAG);
    else
      setFlag(cpu.p & ~(CARRY_FLAG));
    val >>= 1;
    setZFlag(val);
    setNFlag(val);
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address -= getAddressIncrement();
    write_mem_b(val, opcode_address);

  }
}
void ORA() {
  opcode_value |= cpu.a;
  setNFlag(opcode_value);
  setZFlag(opcode_value);
  cpu.a = opcode_value;
}

void ROL() {
  u8 cflag = cpu.p & CARRY_FLAG;
  ASL();
  u8 newVal;
  if (address_mode == ACCUMULATOR) {
    cpu.a = cpu.a | (cflag != 0);
    newVal = cpu.a;
  } else {
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address -= getAddressIncrement();
    write_mem_b(mem.buffer[opcode_address] | (cflag != 0), opcode_address);
    newVal = mem.buffer[opcode_address];
  }
  setZFlag(newVal);
  setNFlag(newVal);
}

void ROR() {
  u8 carry = cpu.p & 0x01;
  if (address_mode == ACCUMULATOR) {
    cpu.p = cpu.p | (cpu.a & 0x01);
    cpu.a >>= 1;
    if (carry == 0x01) cpu.a |= 0x80;
    setZFlag(cpu.a);
    setNFlag(cpu.a);
  } else {
    cpu.p = cpu.p | (opcode_value & 0x01);
    opcode_value >>= 1;
    if (carry == 0x01) opcode_value |= 0x80;
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address -= getAddressIncrement();
    write_mem_b(opcode_value, opcode_address);
    setZFlag(opcode_value);
    setNFlag(opcode_value);
  }
}

void ADC() {
  short val = (short)opcode_value;
  u16 sum = val + (short)cpu.a + ((cpu.p & CARRY_FLAG) != 0);
  // set carry flag
  if ((sum & 0x00FF) != 0x00FF && (sum & 0x0100) == 0x0100)
    setFlag(cpu.p | CARRY_FLAG);
  else
    setFlag(cpu.p & ~(CARRY_FLAG));
  setZFlag((u8)(sum & 0x0ff));
  setNFlag((u8)(sum & 0xff));
  setVFlag(cpu.a, opcode_value, (u8)(sum & 0x0FF));
  cpu.a = (u8)(sum & 0x0ff);
}

void DEC() {
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address -= getAddressIncrement();
  opcode_value--;
  write_mem_b(opcode_value, opcode_address);
  setZFlag(opcode_value);
  setNFlag(opcode_value);
}
void DEX() {
  cpu.x--;
  setZFlag(cpu.x);
  setNFlag(cpu.x);
}
void DEY() {
  cpu.y--;
  setZFlag(cpu.y);
  setNFlag(cpu.y);
}
void INC() {
  if (opcode_address == PPU_R_DATA_ADDRESS) ppu.ppu_address-= getAddressIncrement();
  opcode_value++;
  write_mem_b(opcode_value, opcode_address);
  setZFlag(opcode_value);
  setNFlag(opcode_value);
}
void INX() {
  cpu.x++;
  setZFlag(cpu.x);
  setNFlag(cpu.x);
}
void INY() {
  cpu.y++;
  setZFlag(cpu.y);
  setNFlag(cpu.y);
}
void SBC() {
  opcode_value = ~opcode_value;
  ADC();
}
void CLC() {
  setFlag(cpu.p & ~(CARRY_FLAG));
}
void CLD() {
  setFlag(cpu.p & ~(DECIMAL_FLAG));
}
void CLI() {
  setFlag(cpu.p & ~(INTERRUPT_FLAG));
}
void CLV() {
  setFlag(cpu.p & ~(OVERFLOW_FLAG));
}
void SEC() {
  setFlag(cpu.p | CARRY_FLAG);
}
void SED() {
  setFlag(cpu.p | DECIMAL_FLAG);
}
void SEI() {
  setFlag(cpu.p | INTERRUPT_FLAG);
}

// MODE           SYNTAX       HEX LEN TIM
// Immediate     CMP #$44      $C9  2   2
// Zero Page     CMP $44       $C5  2   3
// Zero Page,X   CMP $44,X     $D5  2   4
// Absolute      CMP $4400     $CD  3   4
// Absolute,X    CMP $4400,X   $DD  3   4+
// Absolute,Y    CMP $4400,Y   $D9  3   4+
// Indirect,X    CMP ($44,X)   $C1  2   6
// Indirect,Y    CMP ($44),Y   $D1  2   5+
void CMP() {
  if (cpu.a == opcode_value) {
    setFlag(cpu.p | CARRY_FLAG);
    setFlag(cpu.p | ZERO_FLAG);
  } else if (cpu.a < opcode_value) {
    setFlag(cpu.p & ~(CARRY_FLAG));
    setFlag(cpu.p & ~(ZERO_FLAG));
  } else {
    setFlag(cpu.p | CARRY_FLAG);
    setFlag(cpu.p & ~(ZERO_FLAG));
  }
  setNFlag(cpu.a + ~(opcode_value) + 1);
}
// MODE           SYNTAX       HEX LEN TIM
// Immediate     CPX #$44      $E0  2   2
// Zero Page     CPX $44       $E4  2   3
// Absolute      CPX $4400     $EC  3   4

void CPX() {
  if (cpu.x == opcode_value) {
    setFlag(cpu.p | CARRY_FLAG);
    setFlag(cpu.p | ZERO_FLAG);
  } else if (cpu.x < opcode_value) {
    setFlag(cpu.p & ~(CARRY_FLAG));
    setFlag(cpu.p & ~(ZERO_FLAG));
  } else {
    setFlag(cpu.p | CARRY_FLAG);
    setFlag(cpu.p & ~(ZERO_FLAG));
  }
  setNFlag(cpu.x + ~(opcode_value) + 1);
}
// MODE           SYNTAX       HEX LEN TIM
// Immediate     CPY #$44      $C0  2   2
// Zero Page     CPY $44       $C4  2   3
// Absolute      CPY $4400     $CC  3   4
void CPY() {
  if (cpu.y == opcode_value) {
    setFlag(cpu.p | CARRY_FLAG);
    setFlag(cpu.p | ZERO_FLAG);
  } else if (cpu.y < opcode_value) {
    setFlag(cpu.p & ~(CARRY_FLAG));
    setFlag(cpu.p & ~(ZERO_FLAG));
  } else {
    setFlag(cpu.p | CARRY_FLAG);
    setFlag(cpu.p & ~(ZERO_FLAG));
  }
  setNFlag(cpu.y + ~(opcode_value) + 1);
}
void PHA() {
  write_mem_b(cpu.a, (u8)cpu.stack-- + STACK_OFFSET);
}
void PHP() {
  write_mem_b(cpu.p | 0x30, cpu.stack-- + STACK_OFFSET);
}
void PLA() {
  cpu.a = read_mem_b((u8)++cpu.stack + STACK_OFFSET);
  setZFlag(cpu.a);
  setNFlag(cpu.a);
}
void PLP() {
  cpu.p = (read_mem_b((u8)++cpu.stack + STACK_OFFSET) & 0xef) | 0x20;

}
// Affects Flags: none
//
// MODE           SYNTAX       HEX LEN TIM
// Absolute      JMP $5597     $4C  3   3
// Indirect      JMP ($5597)   $6C  3   5
void JMP() {
  if (address_mode == ABSOLUTE)
    cpu.pc = opcode_address;
  else {
    if ((opcode_address & 0x00FF) == 0x00FF) {
      cpu.pc = ((u16)(read_mem_b(opcode_address & 0xFF00)) << 8) |
               read_mem_b(opcode_address);
    } else
      cpu.pc = read_mem_w(opcode_address);
  }
}
void JSR() {
  // 3 byte
  u16 t = cpu.pc + 3;
  // minus 1
  t -= 1;
  write_mem_b((t >> 8) & 0xFF, cpu.stack-- + STACK_OFFSET);
  write_mem_b(t & 0xFF, cpu.stack-- + STACK_OFFSET);
  cpu.pc = opcode_address;
}
void RTI() {
  cpu.p = (read_mem_b((u8)++cpu.stack + STACK_OFFSET) & 0xef) | 0x20;
  u16 lowByte = read_mem_b(++cpu.stack + STACK_OFFSET);
  u16 heightByte = read_mem_b(++cpu.stack + STACK_OFFSET);
  cpu.pc = (heightByte << 8) | lowByte;
}
void RTS() {
  u16 lowByte = read_mem_b(++cpu.stack + STACK_OFFSET);
  u16 heightByte = read_mem_b(++cpu.stack + STACK_OFFSET);
  cpu.pc = (heightByte << 8) | lowByte;
  cpu.pc += 1;
}
// MNEMONIC                       HEX
// BPL (Branch on PLus)           $10
// BMI (Branch on MInus)          $30
// BVC (Branch on oVerflow Clear) $50
// BVS (Branch on oVerflow Set)   $70
// BCC (Branch on Carry Clear)    $90
// BCS (Branch on Carry Set)      $B0
// BNE (Branch on Not Equal)      $D0
// BEQ (Branch on EQual)          $F0
byte BCC() {
  u8 flag = getCflag();
  if (flag != CARRY_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte BCS() {
  u8 flag = getCflag();
  if (flag == CARRY_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte BEQ() {
  if (getZflag() == ZERO_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte BMI() {
  if (getNflag() == NEGATIVE_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte BNE() {
  if (getZflag() != ZERO_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte BPL() {
  if (getNflag() != NEGATIVE_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte BVC() {
  if (getVflag() != OVERFLOW_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte BVS() {
  if (getVflag() == OVERFLOW_FLAG) {
    byte val = (byte)opcode_value;
    cpu.pc += val + 2;
    return 1;
  }
  return -1;
}
byte emulate() {
  opcode_address = 0;
  opcode_value = 0;
  lastCycles = cpu.cycles;
  if (cpu.nmi_interrupt != 0) {
    cpu.nmi_interrupt = 0;
    interrupt_nmi();
    cpu.cycles += 7;
    return 1;
  }
  u8 opcode = mem.buffer[cpu.pc];
  //printf("%04x%5s", cpu.pc, "");
  //if (opcodes_size[(u8)opcode] == 1) {
  //  printf("%s", opcode_name[(u8)opcode]);
  //} else if (opcodes_size[(u8)opcode] == 2) {
  //  printf(opcode_name[(u8)opcode], mem.buffer[cpu.pc + 1]);
  //} else if (opcodes_size[(u8)opcode] == 3) {
  //  printf(opcode_name[(u8)opcode], mem.buffer[cpu.pc + 2],
  //         mem.buffer[cpu.pc + 1]);
  //} else
  //  printf("cant find it \n");
  //printf("%-*s", 25, "");
  //// showRegisters();
  //printf("cycles: %u ppuCycles: %u scanlines: %u", cpu.cycles, ppu.cycles, ppu.scanlines);
  //printf("\n");
  u8 r;
  switch (opcode) {
      // LDA
    case 0xA1:
      index_indirect();
      LDA();
      cpu.cycles += 6;
      break;
    case 0xA5:
      zero_page();
      LDA();
      cpu.cycles += 3;
      break;
    case 0xA9:
      immediate();
      LDA();
      cpu.cycles += 2;
      break;
    case 0xAD:
      absolute();
      LDA();
      cpu.cycles += 4;
      break;
    case 0xB1: {
      r = indirect_indexed();
      LDA();
      cpu.cycles += 5;
      cpu.cycles += r;
    } break;
    case 0xB5:
      zero_page_x();
      LDA();
      cpu.cycles += 4;
      break;
    case 0xB9:
      r = absolute_y();
      LDA();
      cpu.cycles += 4;
      cpu.cycles += r;

      break;
    case 0xBD:
      r = absolute_x();
      LDA();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    // LDX
    case 0xA2:
      immediate();
      LDX();
      cpu.cycles += 2;
      break;
    case 0xA6:
      zero_page();
      LDX();
      cpu.cycles += 3;
      break;
    case 0xB6:
      zero_page_y();
      LDX();
      cpu.cycles += 4;
      break;
    case 0xAE:
      absolute();
      LDX();
      cpu.cycles += 4;
      break;
    case 0xBE:
      r = absolute_y();
      LDX();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
      // LDY
    case 0xA0:
      immediate();
      LDY();
      cpu.cycles += 2;
      break;
    case 0xA4:
      zero_page();
      LDY();
      cpu.cycles += 3;
      break;
    case 0xB4:
      zero_page_x();
      LDY();
      cpu.cycles += 4;
      break;
    case 0xAC:
      absolute();
      LDY();
      cpu.cycles += 4;
      break;
    case 0xBC:
      r = absolute_x();
      LDY();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    // STA
    case 0x85:
      zero_page();
      STA();
      cpu.cycles += 3;
      break;
    case 0x95:
      zero_page_x();
      STA();
      cpu.cycles += 4;
      break;
    case 0x8D:
      absolute();
      STA();
      cpu.cycles += 4;
      break;
    case 0x9D:
      absolute_x();
      STA();
      cpu.cycles += 5;
      break;
    case 0x99:
      absolute_y();
      STA();
      cpu.cycles += 5;
      break;
    case 0x81:
      index_indirect();
      STA();
      cpu.cycles += 6;
      break;
    case 0x91:
      indirect_indexed();
      STA();
      cpu.cycles += 6;
      break;
    // STX
    case 0x86:
      zero_page();
      STX();
      cpu.cycles += 3;
      break;
    case 0x96:
      zero_page_y();
      STX();
      cpu.cycles += 4;
      break;
    case 0x8E:
      absolute();
      STX();
      cpu.cycles += 4;
      break;
    // STY
    case 0x84:
      zero_page();
      STY();
      cpu.cycles += 3;
      break;
    case 0x94:
      zero_page_x();
      STY();
      cpu.cycles += 4;
      break;
    case 0x8C:
      absolute();
      STY();
      cpu.cycles += 4;
      break;
    // TAX
    case 0xAA:
      implied();
      TAX();
      cpu.cycles += 2;
      break;
    // TAY
    case 0xA8:
      implied();
      TAY();
      cpu.cycles += 2;
      break;
    // TSX
    case 0xBA:
      implied();
      TSX();
      cpu.cycles += 2;
      break;
    // TXA
    case 0x8A:
      implied();
      TXA();
      cpu.cycles += 2;
      break;
      // TXS
    case 0x9A:
      implied();
      TXS();
      cpu.cycles += 2;
      break;
      // TYA
    case 0x98:
      implied();
      TYA();
      cpu.cycles += 2;
      break;
      // ADC
    case 0x69:
      immediate();
      ADC();
      cpu.cycles += 2;
      break;
    case 0x65:
      zero_page();
      ADC();
      cpu.cycles += 3;
      break;
    case 0x75:
      zero_page_x();
      ADC();
      cpu.cycles += 4;
      break;
    case 0x6D:
      absolute();
      ADC();
      cpu.cycles += 4;
      break;
    case 0x7D:
      r = absolute_x();
      ADC();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x79:
      r = absolute_y();
      ADC();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x61:
      index_indirect();
      ADC();
      cpu.cycles += 6;
      break;
    case 0x71:
      r = indirect_indexed();
      ADC();
      cpu.cycles += 5;
      cpu.cycles += r;
      break;
      // DEC
    case 0xC6:
      zero_page();
      DEC();
      cpu.cycles += 5;
      break;
    case 0xD6:
      zero_page_x();
      DEC();
      cpu.cycles += 6;
      break;
    case 0xCE:
      absolute();
      DEC();
      cpu.cycles += 6;
      break;
    case 0xDE:
      absolute_x();
      DEC();
      cpu.cycles += 7;
      break;
      // DEX
    case 0xCA:
      implied();
      DEX();
      cpu.cycles += 2;
      break;
    // DEY
    case 0x88:
      implied();
      DEY();
      cpu.cycles += 2;
      break;
      // INC - Increment M by One
      // INX - Increment X by One
      // INY - Increment Y by One
      // SBC - Subtract M from A with Borrow
      // INC
    case 0xE6:
      zero_page();
      INC();
      cpu.cycles += 5;
      break;
    case 0xF6:
      zero_page_x();
      INC();
      cpu.cycles += 6;
      break;
    case 0xEE:
      absolute();
      INC();
      cpu.cycles += 6;
      break;
    case 0xFE:
      absolute_x();
      INC();
      cpu.cycles += 7;
      break;
      // INX
    case 0xE8:
      implied();
      INX();
      cpu.cycles += 2;
      break;
      // INY
    case 0xC8:
      implied();
      INY();
      cpu.cycles += 2;
      break;
    // SBC
    case 0xE9:
      immediate();
      SBC();
      cpu.cycles += 2;
      break;
    case 0xE5:
      zero_page();
      SBC();
      cpu.cycles += 3;
      break;
    case 0xF5:
      zero_page_x();
      SBC();
      cpu.cycles += 4;
      break;
    case 0xED:
      absolute();
      SBC();
      cpu.cycles += 4;
      break;
    case 0xFD:
      r = absolute_x();
      SBC();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0xF9:
      r = absolute_y();
      SBC();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0xE1:
      index_indirect();
      SBC();
      cpu.cycles += 6;
      break;
    case 0xF1:
      r = indirect_indexed();
      SBC();
      cpu.cycles += 5;
      cpu.cycles += r;
      break;
      // and
    case 0x29:
      immediate();
      AND();
      cpu.cycles += 2;
      break;
    case 0x25:
      zero_page();
      AND();
      cpu.cycles += 3;
      break;
    case 0x35:
      zero_page_x();
      AND();
      cpu.cycles += 4;
      break;
    case 0x2D:
      absolute();
      AND();
      cpu.cycles += 4;
      break;
    case 0x3D:
      r = absolute_x();
      AND();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x39:
      r = absolute_y();
      AND();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x21:
      index_indirect();
      AND();
      cpu.cycles += 6;
      break;
    case 0x31:
      r = indirect_indexed();
      AND();
      cpu.cycles += 5;
      cpu.cycles += r;
      break;
      // ASL
    case 0x0A:
      address_mode = ACCUMULATOR;
      ASL();
      cpu.cycles += 2;
      break;
    case 0x06:
      zero_page();
      ASL();
      cpu.cycles += 5;
      break;
    case 0x16:
      zero_page_x();
      ASL();
      cpu.cycles += 6;
      break;
    case 0x0E:
      absolute();
      ASL();
      cpu.cycles += 6;
      break;
    case 0x1E:
      absolute_x();
      ASL();
      cpu.cycles += 7;
      break;
    // BIT
    case 0x24:
      zero_page();
      BIT();
      cpu.cycles += 3;
      break;
    case 0x2C:
      absolute();
      BIT();
      cpu.cycles += 4;
      break;
    // EOR
    case 0x49:
      immediate();
      EOR();
      cpu.cycles += 2;
      break;
    case 0x45:
      zero_page();
      EOR();
      cpu.cycles += 3;
      break;
    case 0x55:
      zero_page_x();
      typedef uint64_t u64;
      EOR();
      cpu.cycles += 4;
      break;
    case 0x4D:
      absolute();
      EOR();
      cpu.cycles += 4;
      break;
    case 0x5D:
      r = absolute_x();
      EOR();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x59:
      r = absolute_y();
      EOR();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x41:
      index_indirect();
      EOR();
      cpu.cycles += 6;
      break;
    case 0x51:
      r = indirect_indexed();
      EOR();
      cpu.cycles += 5;
      cpu.cycles += r;
      break;
      // LSR
    case 0x4A:
      address_mode = ACCUMULATOR;
      LSR();
      cpu.cycles += 2;
      break;
    case 0x46:
      zero_page();
      LSR();
      cpu.cycles += 5;
      break;
    case 0x56:
      zero_page_x();
      LSR();
      cpu.cycles += 6;
      break;
    case 0x4E:
      absolute();
      LSR();
      cpu.cycles += 6;
      break;
    case 0x5E:
      absolute_x();
      LSR();
      cpu.cycles += 7;
      break;
    // ORA
    case 0x09:
      immediate();
      ORA();
      cpu.cycles += 2;
      break;
    case 0x05:
      zero_page();
      ORA();
      cpu.cycles += 3;
      break;
    case 0x15:
      zero_page_x();
      ORA();
      cpu.cycles += 4;
      break;
    case 0x0D:
      absolute();
      ORA();
      cpu.cycles += 4;
      break;
    case 0x1D:
      r = absolute_x();
      ORA();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x19:
      r = absolute_y();
      ORA();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0x01:
      index_indirect();
      ORA();
      cpu.cycles += 6;
      break;
    case 0x11:
      r = indirect_indexed();
      ORA();
      cpu.cycles += 5;
      cpu.cycles += r;
      break;
      // ROL
    case 0x2A:
      address_mode = ACCUMULATOR;
      ROL();
      cpu.cycles += 2;
      break;
    case 0x26:
      zero_page();
      ROL();
      cpu.cycles += 5;
      break;
    case 0x36:
      zero_page_x();
      ROL();
      cpu.cycles += 6;
      break;
    case 0x2E:
      absolute();
      ROL();
      cpu.cycles += 6;
      break;
    case 0x3E:
      absolute_x();
      ROL();
      cpu.cycles += 7;
      break;
      // ROR
    case 0x6A:
      address_mode = ACCUMULATOR;
      ROR();
      cpu.cycles += 2;
      break;
    case 0x66:
      zero_page();
      ROR();
      cpu.cycles += 5;
      break;
    case 0x76:
      zero_page_x();
      ROR();
      cpu.cycles += 6;
      break;
    case 0x6E:
      absolute();
      ROR();
      cpu.cycles += 6;
      break;
    case 0x7E:
      absolute_x();
      ROR();
      cpu.cycles += 7;
      break;
      // bcc
    case 0x90:
      relative();
      if (BCC() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    case 0xB0:
      relative();
      if (BCS() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    case 0xF0:
      relative();
      if (BEQ() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    case 0x30:
      relative();
      if (BMI() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    case 0xD0:
      relative();
      if (BNE() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    case 0x10:
      relative();
      if (BPL() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    case 0x50:
      relative();
      if (BVC() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    case 0x70:
      relative();
      if (BVS() == 1) {
        cpu.cycles += 3;
        return 1;
      }
      cpu.cycles += 2;
      break;
    // JMP
    case 0x4C:
      absolute();
      JMP();
      cpu.cycles += 3;
      return 1;
    case 0x6C:
      indirect();
      JMP();
      cpu.cycles += 5;
      return 1;
      // JSR
    case 0x20:
      absolute();
      JSR();
      cpu.cycles += 6;
      return 1;
      // RTI
    case 0x40:
      implied();
      RTI();
      cpu.cycles += 6;
      return 1;
      // RTS
    case 0x60:
      implied();
      RTS();
      cpu.cycles += 6;
      return 1;
      // CLC
    case 0x18:
      implied();
      CLC();
      cpu.cycles += 2;
      break;
    case 0xD8:
      implied();
      CLD();
      cpu.cycles += 2;
      break;
      // CLI
    case 0x58:
      implied();
      CLI();
      cpu.cycles += 2;
      break;
      // CLV
    case 0xB8:
      implied();
      CLV();
      cpu.cycles += 2;
      break;
      // CMP
    case 0xC9:
      immediate();
      CMP();
      cpu.cycles += 2;
      break;
    case 0xC5:
      zero_page();
      CMP();
      cpu.cycles += 3;
      break;
    case 0xD5:
      zero_page_x();
      CMP();
      cpu.cycles += 4;
      break;
    case 0xCD:
      absolute();
      CMP();
      cpu.cycles += 4;
      break;
    case 0xDD:
      r = absolute_x();
      CMP();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0xD9:
      r = absolute_y();
      CMP();
      cpu.cycles += 4;
      cpu.cycles += r;
      break;
    case 0xC1:
      index_indirect();
      CMP();
      cpu.cycles += 6;
      break;
    case 0xD1:
      r = indirect_indexed();
      CMP();
      cpu.cycles += 5;
      cpu.cycles += r;
      break;
      // CPX
    case 0xE0:
      immediate();
      CPX();
      cpu.cycles += 2;
      break;
    case 0xE4:
      zero_page();
      CPX();
      cpu.cycles += 3;
      break;
    case 0xEC:
      absolute();
      CPX();
      cpu.cycles += 4;
      break;
      // CPY
    case 0xC0:
      immediate();
      CPY();
      cpu.cycles += 2;
      break;
    case 0xC4:
      zero_page();
      CPY();
      cpu.cycles += 3;
      break;
    case 0xCC:
      absolute();
      CPY();
      cpu.cycles += 4;
      break;
      // SEC
    case 0x38:
      implied();
      SEC();
      cpu.cycles += 2;
      break;
      // SED
    case 0xF8:
      implied();
      SED();
      cpu.cycles += 2;
      break;
      // SEI
    case 0x78:
      implied();
      SEI();
      cpu.cycles += 2;
      break;
      // PHA
    case 0x48:
      implied();
      PHA();
      cpu.cycles += 3;
      break;
      // PHP
    case 0x08:
      implied();
      PHP();
      cpu.cycles += 3;
      break;
      // PLA
    case 0x68:
      implied();
      PLA();
      cpu.cycles += 4;
      break;
      // PLP
    case 0x28:
      implied();
      PLP();
      cpu.cycles += 4;
      break;
    case 0x00:
      implied();
      interrupt_irq();
      cpu.cycles += 7;
      return 1;
      // nop
    case 0xEA:
    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA:
      implied();
      NOP();
      cpu.cycles += 2;
      break;
    default:
      //showRegisters(&cpu, &ppu);
      cpu.pc += opcodes_size[opcode];
      return -1;
  }
  cpu.pc += opcodes_size[opcode];
  return 1;
}
