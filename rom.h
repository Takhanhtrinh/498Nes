#ifndef ROOM_H
#define ROOM_H
#include "definition.h"
#include "mem.h"
#include "ppu.h"
// use for read room
#define HEADER_SIZE 0x0F
#define PRG_ADDRESS 0x8000
#define HORIZONTAL_MIRROR 0
#define VERTICAL_MIRROR 1
typedef struct {
  // "NES1"
  char fileFormat[4];
  // number of 16kb for program code
  u8 size_prg;
  // number of 8kb for character
  u8 size_chr;
  //  ROM Control Byte 1:
  //• Bit 0 - Indicates the type of mirroring used by the game
  // where 0 indicates horizontal mirroring, 1 indicates
  // vertical mirroring.
  //• Bit 1 - Indicates the presence of battery-backed RAM at
  // memory locations $6000-$7FFF.
  //• Bit 2 - Indicates the presence of a 512-byte trainer at
  // memory locations $7000-$71FF.
  //• Bit 3 - If this bit is set it overrides bit 0 to indicate fourscreen
  // mirroring should be used.
  //• Bits 4-7 - Four lower bits of the mapper number.
  u8 byte_6;

  //  ROM Control Byte 2:
  //• Bits 0-3 - Reserved for future usage and should all be 0.
  //• Bits 4-7 - Four upper bits of the mapper number.
  u8 byte_7;
  // Number of 8 KB RAM banks. For compatibility with previous
  // versions of the iNES format, assume 1 page of RAM when
  // this is 0.
  u8 byte_8;
} INES;
extern INES ines;
int read_room(const char* filePath);
static inline u8 getMapper() {
  return ines.byte_6 >> 4;
} 
static inline u8 getMirror() {
  return ines.byte_6 & 0x01;
}
static inline u8 getTrainer() {
  return ines.byte_6 & 0x04;
}

#endif
