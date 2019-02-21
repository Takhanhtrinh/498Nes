#ifndef PPU_H
#define PPU_H
#include <assert.h>
#include "color.h"
#include "cpu.h"
#include "definition.h"
#include "mem.h"
#include "rom.h"
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 256
#define PPU_SIZE 0x4000
#define OAM_SIZE 0x100
#define PPU_CTR_ADDRESS 0x2000
#define PPU_MASK_ADDRESS 0x2001

#define CLOCK_PER_SCANLINE 341

#define TOP_LEFT_SCREEN 0x2000
#define TOP_RIGHT_SCREEN 0x2400
#define BOTTOM_LEFT_SCREEN 0x2800
#define BOTTOM_RIGHT_SCREEN 0x2C00

#define NAMETABLE_ROW 30
#define NAMETABLE_COL 32

// ppu_ctrl masking
// 7  bit  0
//---- ----
// VPHB SINN
//|||| ||||
//|||| ||++- Base nametable address
//|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
//|||| |+--- VRAM address increment per CPU read/write of PPUDATA
//|||| |     (0: add 1, going across; 1: add 32, going down)
//|||| +---- Sprite pattern table address for 8x8 sprites
//||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
//|||+------ Background pattern table address (0: $0000; 1: $1000)
//||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
//|+-------- PPU master/slave select
//|          (0: read backdrop from EXT pins; 1: output color on EXT pins)
//+--------- Generate an NMI at the start of the
//           vertical blanking interval (0: off; 1: on)
#define NAME_TABLE_ADDRESS_MASK 0x03
#define PPU_ADDRESS_INCREMENT_MASK 0x04
#define SPRITE_PATTERN_ADDRESS_MASK 0x08
#define BACKGROUND_PATTERN_ADDRESS_MASK 0x10
#define SPRINT_SIZE_MASK 0x20
#define NMI_MASK 0x80
#define SPRITE_8X8 0x00
#define SPRITE_16X8 0x01;

// PPU_STATUS masking
// 7  bit  0
//---- ----
// VSO. ....
//|||| ||||
//|||+-++++- Least significant bits previously written into a PPU register
//|||        (due to register not being updated for this address)
//||+------- Sprite overflow. The intent was for this flag to be set
//||         whenever more than eight sprites appear on a scanline, but a
//||         hardware bug causes the actual behavior to be more complicated
//||         and generate false positives as well as false negatives; see
//||         PPU sprite evaluation. This flag is set during sprite
//||         evaluation and cleared at dot 1 (the second dot) of the
//||         pre-render line.
//|+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
//|          a nonzero background pixel; cleared at dot 1 of the pre-render
//|          line.  Used for raster timing.
//+--------- Vertical blank has started (0: not in vblank; 1: in vblank).
//           Set at dot 1 of line 241 (the line *after* the post-render
//           line); cleared after reading $2002 and at dot 1 of the
//           pre-render line.
#define SPRITE_OVERFLOW_MASK 0x20
#define SPRITE_HIT 0x40
#define VBLANK_OCCUR 0x80
// PPU_MASK masking
// 7  bit  0
// BGRs bMmG
//|||| ||||
//|||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
//|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
//|||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
//|||| +---- 1: Show background
//|||+------ 1: Show sprites
//||+------- Emphasize red
//|+-------- Emphasize green
//+--------- Emphasize blue
#define GRAY_SCALE_MASK 0x01
#define BACKGROUND_8PX_LEFTMOST 0x02
#define SPRITE_8PX_LEFTMOST 0x04
#define SHOW_BACKGROUND 0x08
#define SHOW_SRPITES 0x10
#define EMPHASIZE_RED 0x20
#define EMPHASIZE_GREEN 0x40
#define EMPHASIZE_BLUE 0x80

// registers  address
// write read
#define PPU_R_CONTROL_ADDRESS 0x2000
#define PPU_R_MASK_ADDRESS 0x2001
#define PPU_R_STATUS_ADDRESS 0x2002
#define PPU_R_OAM_ADDRESS 0x2003
#define PPU_R_OAM_DATA_ADDRESS 0x2004  // OAMDATA
#define PPU_R_SCROLL_ADDRESS 0x2005    // SCROLL
#define PPU_R_ADDRESS_ADDRESS 0x2006   // address
#define PPU_R_DATA_ADDRESS 0x2007      // data
#define PPU_R_OAM_DMA_ADDRESS 0x4014   // OAM DMA

#define PRE_RENDER 0x01
#define RENDER 0x02
#define POST_RENDER 0x04
#define VBLANK 0x08

#define NUM_PRE_RENDER 261
#define NUM_RENDER 240
#define NUM_POST_RENDER 240
#define NUM_VBLANK 260
#define TOTAL_SCANLINE 262

typedef struct {
  u8 y;
  u8 tile;
  u8 attribute;
  u8 x;
} Sprite;

//-------------
// nametable 2 |
//$2800       |
//-------------
typedef struct {
  uint64_t frame;
  u8 lowByte;
  u8 highByte;
  u8 attributeByte;
  u8 nametableByte;

  Sprite sprites[8];
  // vram
  u8 buffer[PPU_SIZE];
  // oam data
  u8 oam_buffer[OAM_SIZE];
  u8 ppu_ctrl;      // $2000
  u8 ppu_mask;      // $2001
  u8 ppu_status;    // $2002
  u8 oam_addr;      // $2003
  u8 ppu_scroll;    // $2005
  u16 ppu_address;  // $2006
  u8 oam_dma;       // $4014

  // ---------------------------------------------------------------------------
  // use for ppu_addr
  // ppu x scroll https://wiki.nesdev.com/w/index.php/PPU_scrolling#Examples
  // toggle
  u8 isFirstWrite;
  u16 temp_address;
  // ---------------------------------------------------------------------------
  u8 internal_buffer;  // for 2007
  u8 xfine;
  u8 y_pos;

  u16 cycles;
  u16 scanlines;

  u32 screen[SCREEN_WIDTH * SCREEN_HEIGHT];
  u8 nametable[NAMETABLE_ROW][NAMETABLE_COL];

} PPU;
extern PPU ppu;
void create_ppu();
static inline u16 getNameTableAddress()
{
  u8 val = (ppu.ppu_ctrl & NAME_TABLE_ADDRESS_MASK);
  if (val == 0)
    return 0x2000;
  else if (val == 1)
    return 0x2400;
  else if (val == 2)
    return 0x2800;
  else if (val == 3)
    return 0x2C00;
  else
    assert(0 && "getNameTableAddress address invalid");
  return 0;
}
static inline u8 getAddressIncrement()
{
  return ((ppu.ppu_ctrl & PPU_ADDRESS_INCREMENT_MASK) >> 2) == 0 ? 1 : 32;
}
static inline u16 getSpritePatternTableAddress()
{
  return ((ppu.ppu_ctrl & SPRITE_PATTERN_ADDRESS_MASK) >> 3) == 0 ? 0x0000
                                                                  : 0x1000;
}
static inline u16 getBackgroundPatternTableAddress()
{
  return ((ppu.ppu_ctrl & BACKGROUND_PATTERN_ADDRESS_MASK) >> 4) == 0 ? 0x0000
                                                                      : 0x1000;
}
static inline u8 getSpriteSize()
{
  return ((ppu.ppu_ctrl & SPRITE_PATTERN_ADDRESS_MASK) >> 5) == 0 ? SPRITE_8X8
                                                                  : SPRITE_16X8;
}
static inline u8 getNMI() { return (ppu.ppu_ctrl & NMI_MASK) >> 7; }
// get register mask bits
static inline u8 getGrayScale() { return (ppu.ppu_mask & GRAY_SCALE_MASK); }
static inline u8 getBackgroundLeftMost()
{
  return (ppu.ppu_mask & BACKGROUND_8PX_LEFTMOST) >> 1;
}
static inline u8 getSpriteLeftMost()
{
  return (ppu.ppu_mask & SPRITE_8PX_LEFTMOST) >> 2;
}
static inline u8 getShowBackground()
{
  return (ppu.ppu_mask & SHOW_BACKGROUND) >> 3;
}
static inline u8 getShowSprite() { return (ppu.ppu_mask & SHOW_SRPITES) >> 4; }
static inline u8 getEmRed() { return (ppu.ppu_mask & EMPHASIZE_RED) >> 5; }
static inline u8 getEmGreen() { return (ppu.ppu_mask & EMPHASIZE_GREEN) >> 6; }
static inline u8 getEmBlue() { return (ppu.ppu_mask & EMPHASIZE_BLUE) >> 7; }
// read and write registers
void writePPUCtr(u8 val);
// PPUMASK
static inline void writePPUMask(u8 val) { ppu.ppu_mask = val; }
static inline void writeOAMaddress(u8 val) { ppu.oam_addr = val; }
// OAMADDR
static inline void writeOAMdata(u8 val)
{
  ppu.oam_buffer[ppu.oam_addr++] = val;
}
// OAMDATA
static inline u8 readOAMdata() { return ppu.oam_buffer[ppu.oam_addr]; }
// PPUADDR
void writePPUaddr(u8 val);
// PPUSCROLL
void writePPUscroll(u8 val);
// PPUDATA
void writePPUdata(u8 val);
u8 readPPUdata();
// PPUSTATUS
u8 readPPUstatus();
// OAMDMA
void writeDMA(u8 val);

//
void ppu_tick(u16 esplased);
u8 getScanlineState();
static inline u16 fetchNameTable()
{
  return 0x2000 | (ppu.ppu_address & 0x0FFF);
}
static inline u16 fetchAttribute()
{
  return 0x23C0 | (ppu.ppu_address & 0x0C00) | ((ppu.ppu_address >> 4) & 0x38) |
         ((ppu.ppu_address >> 2) & 0x07);
}

static inline u16 getBGcolorAddress(u8 val) { return 0x3F00 + (val * 4); }
static inline u16 getSpriteColorAddress(u8 patternIndex, u8 value)
{
  return 0x3F10 + (patternIndex * 4) + value;
}
void fetchLowByte();
void fetchHighByte();
void printNameTable();
void generatePixel();

void horizontalIncrement();
void verticalIncrement();
u16 ppu_getMirrorAddress(u16 address);
u16 getScreenMirror(u16 address);
void fetchTile();
void updateRegisters(u8 state);

void spriteEvaluation();
void generateSpritePixel(Sprite* sprite, u8 row);

#endif
