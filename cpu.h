#ifndef CPU_H
#define CPU_H
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "definition.h"
#include "helper.h"
#include "mem.h"
#include "ppu.h"
#define IMPLIED 0x01
#define ACCUMULATOR 0x02
#define IMMEDIATE 0x03
#define ZERO_PAGE 0x04
#define ZERO_PAGE_X 0x05
#define ZERO_PAGE_Y 0x06
#define RELATIVE 0x07
#define ABSOLUTE 0x08
#define ABSOLUTE_X 0x09
#define ABSOLUTE_Y 0x0A
#define INDIRECT 0x0B

#define IN_INDIRECT 0x0C
#define IN_INDEXED 0X0D
#define STACK_OFFSET 0x0100

static const char *opcode_name[] = {"BRK",
                                    "ORA ($%02x,x)",
                                    "STP",
                                    "SLO ($%02x,x)",
                                    "NOP $%02x",
                                    "ORA $%02x",
                                    "ASL $%02x",
                                    "SLO $%02x",
                                    "PHP",
                                    "ORA #%02x",
                                    "ASL",
                                    "ANC #%02x",
                                    "NOP $%02x%02x",
                                    "ORA $%02x%02x",
                                    "ASL $%02x%02x",
                                    "SLO $%02x%02x",
                                    "BPL $%02x",
                                    "ORA ($%02x),y",
                                    "STP",
                                    "SLO ($%02x),y",
                                    "NOP $%02x, x",
                                    "ORA $%02x, x",
                                    "ASL $%02x, x",
                                    "SLO $%02x, x",
                                    "CLC",
                                    "ORA $%02x%02x ,y",
                                    "NOP",
                                    "SLO $%02x%02x ,y",
                                    "NOP $%02x%02x ,x",
                                    "ORA $%02x%02x ,x",
                                    "ASL $%02x%02x ,x",
                                    "SLO $%02x%02x ,x",
                                    "JSR $%02x%02x",
                                    "AND ($%02x,x)",
                                    "STP",
                                    "RLA ($%02x, x)",
                                    "BIT $%02x",
                                    "AND $%02x",
                                    "ROL $%02x",
                                    "RLA $%02x",
                                    "PLP",
                                    "AND #%02x",
                                    "ROL",
                                    "ANC #%02x",
                                    "BIT $%02x%02x",
                                    "AND $%02x%02x",
                                    "ROL $%02x%02x",
                                    "RLA $%02x%02x",
                                    "BMI $%02x",
                                    "AND ($%02x), y",
                                    "STP",
                                    "RLA ($%02x), y",
                                    "NOP $%02x, x",
                                    "AND $%02x, x",
                                    "ROL $%02x, x",
                                    "RLA $%02x, x",
                                    "SEC",
                                    "AND $%02x%02x ,y",
                                    "NOP",
                                    "RLA $%02x%02x ,y",
                                    "NOP $%02x%02x ,x",
                                    "AND $%02x%02x ,x",
                                    "ROL $%02x%02x ,x",
                                    "RLA $%02x%02x ,x",
                                    "RTI",
                                    "EOR ($%02x,x)",
                                    "STP",
                                    "SRE ($%02x,x)",
                                    "NOP $%02x",
                                    "EOR $%02x",
                                    "LSR $%02x",
                                    "SRE $%02x",
                                    "PHA",
                                    "EOR #%02x",
                                    "LSR",
                                    "ALR #%02x",
                                    "JMP $%02x%02x",
                                    "EOR $%02x%02x",
                                    "LSR $%02x%02x",
                                    "SRE $%02x%02x",
                                    "BVC $%02x",
                                    "EOR ($%02x),y",
                                    "STP",
                                    "SRE ($%02x), y",
                                    "NOP $%02x,x",
                                    "EOR $%02x,x",
                                    "LSR $%02x,x",
                                    "SRE $%02x,x",
                                    "CLI",
                                    "EOR $%02x%02x ,y",
                                    "NOP",
                                    "SRE $%02x%02x ,y",
                                    "NOP $%02x%02x ,x",
                                    "EOR $%02x%02x ,x",
                                    "LSR $%02x%02x ,x",
                                    "SRE $%02x%02x ,x",
                                    "RTS",
                                    "ADC ($%02x,x)",
                                    "STP",
                                    "RRA ($%02x,x)",
                                    "NOP $%02x",
                                    "ADC $%02x",
                                    "ROR $%02x",
                                    "RRA $%02x",
                                    "PLA",
                                    "ADC #%02x",
                                    "ROR",
                                    "ARR #%02x",
                                    "JMP ($%02x%02x)",
                                    "ADC $%02x%02x",
                                    "ROR $%02x%02x",
                                    "RRA $%02x%02x",
                                    "BVS $%02x",
                                    "ADC ($%02x), y",
                                    "STP",
                                    "RRA ($%02x), y",
                                    "NOP $%02x, x",
                                    "ADC $%02x,x",
                                    "ROR $%02x,x",
                                    "RRA $%02x,x",
                                    "SEI",
                                    "ADC $%02x%02x ,y",
                                    "NOP",
                                    "RRA $%02x%02x ,y",
                                    "NOP $%02x%02x ,x",
                                    "ADC $%02x%02x ,x",
                                    "ROR $%02x%02x ,x",
                                    "RRA $%02x%02x ,x",
                                    "NOP #%02x",
                                    "STA ($%02x,x)",
                                    "NOP #%02x",
                                    "SAX ($%02x,x)",
                                    "STY $%02x",
                                    "STA $%02x",
                                    "STX $%02x",
                                    "SAX $%02x",
                                    "DEY",
                                    "NOP #%02x",
                                    "TXA",
                                    "XXA #%02x",
                                    "STY $%02x%02x",
                                    "STA $%02x%02x",
                                    "STX $%02x%02x",
                                    "SAX $%02x%02x",
                                    "BBC $%02x",
                                    "STA ($%02x),y",
                                    "STP",
                                    "AHX ($%02x),y",
                                    "STY $%02x,x",
                                    "STA $%02x,x",
                                    "STX $%02x,x",
                                    "SASX $%02x,y",
                                    "TYA",
                                    "STA $%02x%02x ,y",
                                    "TXS",
                                    "TAS $%02x%02x ,y",
                                    "SHY $%02x%02x ,x",
                                    "STA $%02x%02x ,x",
                                    "SHX $%02x%02x ,y",
                                    "AHX $%02x%02x ,y",
                                    "LDY #%02x",
                                    "LDA ($%02x,x)",
                                    "LDX #%02x",
                                    "LAX ($%02x,x)",
                                    "LDY $%02x",
                                    "LDA $%02x",
                                    "LDX $%02x",
                                    "LAX $%02x",
                                    "TAY",
                                    "LDA #%02x",
                                    "TAX",
                                    "LAX #%02x",
                                    "LDY $%02x%02x",
                                    "LDA $%02x%02x",
                                    "LDX $%02x%02x",
                                    "LAX $%02x%02x",
                                    "BCS $%02x",
                                    "LDA ($%02x),y",
                                    "STP",
                                    "LAX ($%02x),y",
                                    "LDY $%02x,x",
                                    "LDA $%02x,x",
                                    "LDX $%02x,y",
                                    "LAX $%02x,y",
                                    "CLV",
                                    "LDA $%02x%02x ,y",
                                    "TSX",
                                    "LAS $%02x%02x ,y",
                                    "LDY $%02x%02x ,x",
                                    "LDA $%02x%02x ,x",
                                    "LDX $%02x%02x ,y",
                                    "LAX $%02x%02x ,y",
                                    "CPY #%02x",
                                    "CMP ($%02x,x)",
                                    "NOP #%02x",
                                    "DCP ($%02x,x)",
                                    "CPY $%02x",
                                    "CMP $%02x",
                                    "DEC $%02x",
                                    "DCP $%02x",
                                    "INY",
                                    "CMP #%02x",
                                    "DEX",
                                    "AXS #%02x",
                                    "CPY $%02x%02x",
                                    "CMP $%02x%02x",
                                    "DEC $%02x%02x",
                                    "DCP $%02x%02x",
                                    "BNE $%02x",
                                    "CMP ($%02x),y",
                                    "STP",
                                    "DCP ($%02x),y",
                                    "NOP $%02x,x",
                                    "CMP $%02x,x",
                                    "DEC $%02x,x",
                                    "DCP $%02x,x",
                                    "CLD",
                                    "CMP $%02x%02x ,y",
                                    "NOP",
                                    "DCP $%02x%02x ,y",
                                    "NOP $%02x%02x ,x",
                                    "CMP $%02x%02x ,x",
                                    "DEC $%02x%02x ,x",
                                    "DCP $%02x%02x ,x",
                                    "CPX #%02x",
                                    "SBC ($%02x,x)",
                                    "NOP #%02x",
                                    "ISC ($%02x,x)",
                                    "CPX $%02x",
                                    "SBC $%02x",
                                    "INC $%02x",
                                    "ISC $%02x",
                                    "INX",
                                    "SBC #%02x",
                                    "NOP",
                                    "SBC #%02x",
                                    "CPX $%02x%02x",
                                    "SBC $%02x%02x",
                                    "INC $%02x%02x",
                                    "ISC $%02x%02x",
                                    "BEQ $%02x",
                                    "SBC ($%02x),y",
                                    "STP",
                                    "ISC ($%02x),y",
                                    "NOP $%02x,x",
                                    "SBC $%02x,x",
                                    "INC $%02x,x",
                                    "ISC $%02x,x",
                                    "SED",
                                    "SBC $%02x%02x ,y",
                                    "NOP",
                                    "ISC $%02x%02x ,y",
                                    "NOP $%02x%02x ,x",
                                    "SBC $%02x%02x ,x",
                                    "INC $%02x%02x ,x",
                                    "ISC $%02x%02x ,x"};
static u8 opcodes_size[] = {
    2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3, 2, 2, 1, 2, 2, 2, 2, 2,
    1, 3, 1, 3, 3, 3, 3, 3, 3, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
    2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3, 1, 2, 1, 2, 2, 2, 2, 2,
    1, 2, 1, 2, 3, 3, 3, 3, 2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
    1, 2, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3, 2, 2, 1, 2, 2, 2, 2, 2,
    1, 3, 1, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
    2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 2, 1, 2, 3, 3, 3, 3, 2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3, 2, 2, 1, 2, 2, 2, 2, 2,
    1, 3, 1, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 3, 3, 3, 3,
    2, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1, 3, 3, 3, 3, 3};
typedef struct {
  u8 nmi_interrupt;
  // program counter
  u16 pc;
  // stack pointer;
  u8 stack;
  // accumulator
  u8 a;
  // index register x
  u8 x;
  // index register y
  u8 y;
  // flag [n v - b d i z c]
  u8 p;
  // cycle 
  u64 cycles;
  
} CPU;
extern CPU cpu;
extern CPU cpu1;
extern u8 address_mode;
extern u8 opcode_value;
extern u16 opcode_address;
extern u64 lastCycles;
// address
void immediate();
void zero_page();
void zero_page_x();
void zero_page_y();
void relative();
void absolute();
u8 absolute_x();
u8 absolute_y();
void indirect();
void index_indirect();
u8 indirect_indexed();
void implied();

byte emulate();

CPU create_cpu();
void setFlag(u8 value);
u8 getNflag();
u8 getVflag();
u8 getZflag();
u8 getCflag();
u8 getIflag();
u8 getDflag();
u8 getBflag();

static inline u8 getPageAcross(u16 address) {
  return ((address & 0x00FF) == 0x00FF) ? 1 : 0;
}

void setNFlag(u8 value);
void setVFlag(u8 value1, u8 value2, u8 result);
void setZFlag(u8 value);
// interrupts
// reset
void interrupt_reset();
// NMI
void interrupt_nmi();
// brk/irq
void interrupt_irq();
// registers
void NOP();  // done

// storage
void LDA();  // done
void LDX();  // done
void LDY();  // done
void STA();  // done
void STX();  // done
void STY();  // done
void TAX();  // done
void TAY();  // done
void TSX();  // done
void TXA();  // done
void TXS();  // done
void TYA();  // done
// bitwise
void AND();  // done
void ASL();  // done
void BIT();
void EOR();  // done
void LSR();  // done
void ORA();  // done
void ROL();  // done
void ROR();  // done
// math
void ADC();  // done
void DEC();  // done
void DEX();  // done
void DEY();  // done
void INC();  // done
void INX();  // done
void INY();  // done
void SBC();  // done
// registers
void CLC();  // done
void CLD();  // done
void CLI();  // done
void CLV();  // done
void SEC();  // done
void SED();  // done
void SEI();  // done
void CMP();  // done
void CPX();  // done
void CPY();  // done
// stack
void PHA();  // done
void PHP();  // done
void PLA();  // done
void PLP();  // done

// Jump
void JMP();  // done
void JSR();  // done
void RTI();  // done
void RTS();  // done
// Branch
//
// BCC - Branch on Carry Clear
// BCS - Branch on Carry Set
// BEQ - Branch on Result Zero
// BMI - Branch on Result Minus
// BNE - Branch on Result not Zero
// BPL - Branch on Result Plus
// BVC - Branch on Overflow Clear
// BVS - Branch on Overflow Set
byte BCC();
byte BCS();
byte BEQ();
byte BMI();
byte BNE();
byte BPL();
byte BVC();
byte BVS();
#endif
