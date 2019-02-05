#ifndef DEFINITION_H
#define DEFINITION_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef uint16_t u16;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;
typedef char byte;
#define CARRY_FLAG 0x01
#define ZERO_FLAG 0x02
#define INTERRUPT_FLAG 0x04
#define DECIMAL_FLAG 0x08
#define BREAK_FLAG 0x10
#define OVERFLOW_FLAG 0x40
#define NEGATIVE_FLAG 0x80
#endif
