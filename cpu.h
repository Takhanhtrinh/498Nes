/*
 * cpu.h
 * header file for cpu emulator
 * by: Trinh Ta
 */
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
#define STACK_OFFSET 0x0100

#define CARRY_FLAG 0x01
#define ZERO_FLAG 0x02
#define INTERRUPT_FLAG 0x04
#define DECIMAL_FLAG 0x08
#define BREAK_FLAG 0x10
#define OVERFLOW_FLAG 0x40
#define NEGATIVE_FLAG 0x80

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
#define INDIRECT_X 0x0C
#define INDIRECT_Y 0X0D
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

/*
 * emulate()
 * This function will get current program counter and decode the instruction.
 * return
 *        number of cycles for the instruction decoded.
 *        return -1 if the instruction is not supported
 */
int emulate();

/*
 * getNFlag()
 * this function will get the Negative flag from flag register p
 * return
 *        1: is on
 *        0: is off
 */
static inline u8 getNFlag() { return cpu.p >> 7; }
/*
 * getVFlag()
 * this function will get the Overflow Flag from Flag register p
 * return
 *        1: is on
 *        0: is off
 */
static inline u8 getVFlag() { return (cpu.p >> 6) & 0x01; }
/*
 * getIFlag()
 * this function will get the Interrupt Flag from Flag register p
 * return
 *        1: is on
 *        0: is off
 */
static inline u8 getIFlag() { return (cpu.p >> 3) & 0x01; }
/*
 * getZFlag()
 * this function will get the Zero Flag from Flag register p
 * return
 *        1: is on
 *        0: is off
 */
static inline u8 getZFlag() { return (cpu.p >> 1) & 0x01; }
/*
 * getCFlag()
 * this function will get the Carry Flag from Flag register p
 * return
 *        1: is on
 *        0: is off
 */
static inline u8 getCFlag() { return cpu.p & 0x01; }

// addressing mode
/*
 * immediate()
 * get immediate value from instruction. the value will be stored in
 * opcode_value as a global variable return none
 */
static inline void immediate() { opcode_address = cpu.pc + 1; }
/*
 * zero_page()
 * get address for zero page. the address will be stored in opcode_address as
 * global variable
 * return
 *        none
 */
void zero_page();

/*
 * zero_page_x()
 * get address for zero page x. the address will be stored in opcode_address as
 * a global variable
 * return
 *        none
 */
void zero_page_x();

/*
 * zero_page_y()
 * get address for zero page y. the address will be stored in opcode_address as
 * a global variable
 * return
 *        none
 */
void zero_page_y();

/*
 * absolute()
 * get address for absolute. the address will be stored in opcode_address as
 * a global variable
 * return
 *        none
 */
void absolute();

/*
 * absolute_x()
 * get address for absolute. the address will be stored in opcode_address as
 * a global variable
 * return
 *        0 for no page across
 *        1 for page across
 */
u8 absolute_x();

/*
 * absolute_y()
 * get address for absolute. the address will be stored in opcode_address as
 * a global variable
 * return
 *        0 for no page across
 *        1 for page across
 */
u8 absolute_y();

/*
 * indirect()
 *
 * return
 *        none
 */
void indirect();

/*
 * indirect_x()
 *
 * return
 *        none
 */
void indirect_x();

/*
 * indirect_y()
 *
 * return
 *        1 if page is a crossed, otherwise return 0
 */
u8 indirect_y();

/*
 * relative()
 * for branch
 * return
 *        none
 */
void relative();

/* isPageAcross
 * check if 2 addresses are in different page
 * parameters:
 *            address1: first address to be compared
 *            address2: second address to be compared
 * return:
 *        0: for 2 addresses are in same page
 *        1: for 2 addresses are in different page
 */
static inline u8 isPageAcross(u16 address1, u16 address2)
{
  return (address1 & 0xFF00) != (address2 & 0xFF00);
}
/*
 * push(u8 val)
 * stack operation, push value to stack
 * parameters:
 *            val: value to push to stack
 * return:
 *          none
 */
void push(u8 val);

/*
 * pull()
 * pull the value from stack
 * return
 *        value from stack
 */
u8 pull();

/*
 * interrupt_nmi()
 * trigger nmi interrupt
 * load nmi vector to program counter
 */
void interrupt_nmi();

/*
 * interrupt_reset()
 * trigger reset interrupt
 * load reset vector to program counter and set cpu.p
 */
void interrupt_reset();

/*
 * interrupt_irq()
 * irq interrupt
 */
void interrupt_irq();

// set the zero flag if the value is zero, otherwise turn it off
static inline void setZ(u8 value)
{
  cpu.p = (value == 0) ? (cpu.p | ZERO_FLAG) : (cpu.p & ~(ZERO_FLAG));
}

// set the negative if the value is negative, otherwise turn it off
static inline void setN(u8 value)
{
  cpu.p = ((value >> 7) == 0x01) ? (cpu.p | NEGATIVE_FLAG) : (cpu.p & ~(NEGATIVE_FLAG));
}

// check if the value meet the condition to turn on the N and Z Flag, otherwise
// the N or Z Flag is off
void setNZ(u8 value);

// add with carry
void ADC();

void AND();

void ASL();

// branch if carry clear
// return 1 if branch is occured, otherwise return 0
u8 BCC();
// branch if carry is set
// return 1 if branch is occured, otherwise return 0
u8 BCS();

// branch if zero is set
u8 BEQ();

// BIT test
void BIT();

// branch if minus
u8 BMI();

// branch if not equal
u8 BNE();

// branch if positive
u8 BPL();

// force interrupt
void BRK();

// branch if overflow clear
u8 BVC();
// branch if overflow set
u8 BVS();

// clear carry flag
static inline void CLC() { cpu.p = cpu.p & ~(CARRY_FLAG); }
// clear decimal
static inline void CLD() { cpu.p = cpu.p & ~(DECIMAL_FLAG); }
// clear interrupt flag
static inline void CLI() { cpu.p = cpu.p & ~(INTERRUPT_FLAG); }
// clear overflow
static inline void CLV() { cpu.p = cpu.p & ~(OVERFLOW_FLAG); }
// comapre with a
void CMP();
// comapre with x
void CPX();
// comapre with y
void CPY();
// decrement memory
void DEC();
// decrement x register
void DEX();
// decrement y register
void DEY();
// exclusive OR
void EOR();
// increment memory
void INC();
// increment X
void INX();
// increment y
void INY();
// jump
static inline void JMP() { cpu.pc = opcode_address; }
// jump with return
void JSR();
// load accumulator
void LDA();
// load x register
void LDX();
// load y register
void LDY();
// logical shift right
void LSR();
// logical inclusive OR
void ORA();
// push accumulator
static inline void PHA() { push(cpu.a); }
// push flag
static inline void PHP() { push(cpu.p | 0x30); }
// pull accumulator
void PLA();
// pull flag
// https://wiki.nesdev.com/w/index.php/Status_flags
// plp ignore bit 4 and 5. nestes.nes bit 5 is always on
static inline void PLP() { cpu.p = (pull() & 0xEF) | (cpu.p & 0x10) | 0x20; }
// rotate left
void ROL();
// rotate right
void ROR();
// return from interrupt
void RTI();
// return from subroutine
void RTS();
// subtract with carry
void SBC();
// set carry flag
static inline void SEC() { cpu.p = cpu.p | CARRY_FLAG; }
// set decimal
static inline void SED() { cpu.p = cpu.p | DECIMAL_FLAG; }
// set interrupt flag
static inline void SEI() { cpu.p = cpu.p | INTERRUPT_FLAG; }
// store Accumulator
void STA();
// store x
void STX();
// store y
void STY();
// transfer accumulator to x
static inline void TAX() { cpu.x = cpu.a; }
// transfer accumulator to y
static inline void TAY() { cpu.y = cpu.a; }
// transfer stack pointer to x
void TSX();
// transfer x to accumulator
void TXA();
// transfer x to stack
static inline void TXS() { cpu.stack = cpu.x; }
// transfer y to accumulator
static inline void TYA() { cpu.a = cpu.y; }
// Opcodes for disassembler
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
#endif
