#include "ppu.h"
#include <unistd.h>
void create_ppu()
{
  ppu.frame = 0;
  for (int i = 0; i < PPU_SIZE; i++) {
    ppu.buffer[i] = 0x00;
  }
  for (int i = 0; i < OAM_SIZE; i++) ppu.oam_buffer[i] = 0x00;
  ppu.ppu_ctrl = 0;
  ppu.ppu_mask = 0;
  ppu.ppu_status = 0;
  ppu.oam_addr = 0;
  ppu.ppu_scroll = 0;
  ppu.ppu_address = 0;
  ppu.isFirstWrite = 1;
  ppu.cycles = 0;
  ppu.scanlines = 0;
  for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
    ppu.screen[i] = 0;
  }
}

u8 getScanlineState()
{
  u16 currentScanline = ppu.scanlines;
  if (currentScanline < NUM_RENDER)
    return RENDER;
  else if (currentScanline == NUM_POST_RENDER)
    return POST_RENDER;
  else if (currentScanline == NUM_PRE_RENDER)
    return PRE_RENDER;
  else
    return VBLANK;
}
// PPUCTRL
void writePPUCtr(u8 val)
{
  ppu.ppu_ctrl = val;
  // set temp address
  ppu.temp_address = (ppu.temp_address & 0xF3FF) | (((u16)(val & 0x03)) << 10);
}
// PPUSCROLL
void writePPUscroll(u8 val)
{
  if (ppu.isFirstWrite == 1) {
    ppu.temp_address = (ppu.temp_address & 0xFFE0) | ((u16)val >> 3);
    ppu.xfine = val & 0x07;
    ppu.isFirstWrite = 0;
  }
  else {
    u16 castVal = (u16)(val);
    ppu.temp_address = (ppu.temp_address & 0x8C1F) |
                       (((castVal & 0x0007) << 12) | ((castVal & 0x00F8) << 2));
    ppu.isFirstWrite = 1;
  }
}
// PPUADDR
void writePPUaddr(u8 val)
{
  if (ppu.isFirstWrite == 1) {
    ppu.temp_address = (ppu.temp_address & 0x80FF) | (((u16)(val & 0x3F)) << 8);
    ppu.isFirstWrite = 0;
  }
  else {
    ppu.temp_address = (ppu.temp_address & 0xFF00) | (u16)val;
    ppu.isFirstWrite = 1;
    ppu.ppu_address = ppu.temp_address;
  }
}
// PPUDATA
void writePPUdata(u8 val)
{
  ppu.ppu_address = ppu_getMirrorAddress(ppu.ppu_address);
  ppu.buffer[ppu.ppu_address] = val;
  ppu.ppu_address += getAddressIncrement();
}
// C. PPU Notes
// ------------
//   Reading and writing to VRAM consists of a multi-step process:

//     Writing to VRAM                    Reading from VRAM
//     ---------------                    -----------------
//     1) Wait for VBlank                 1) Wait for VBlank
//     2) Write upper VRAM address        2) Write upper VRAM address
//        byte into $2006                    byte into $2006
//     3) Write lower VRAM address        3) Write lower VRAM address
//        byte into $2006                    byte into $2006
//     4) Write data to $2007             4) Read $2007 (invalid data once)
//                                        5) Read data from $2007

// NOTE: Step #4 when reading VRAM is only necessary when reading
// VRAM data not in the $3F00-3FFF range.
// PPUDATA
u8 readPPUdata()
{
  u8 oldData = ppu.internal_buffer;
  ppu.ppu_address = ppu_getMirrorAddress(ppu.ppu_address);
  u8 newData = ppu.buffer[ppu.ppu_address];
  ppu.ppu_address += getAddressIncrement();
  if (ppu.ppu_address < 0x3F00) {
    ppu.internal_buffer = newData;
    return oldData;
  }
  else
    return newData;
}
// PPUSTATUS
u8 readPPUstatus()
{
  u8 r = ppu.ppu_status;
  ppu.ppu_status &= ~(VBLANK_OCCUR);
  ppu.isFirstWrite = 1;
  return r;
}
void writeDMA(u8 val)
{
  u16 address = val * 0x100;
  for (int i = 0; i < 0x100; i++) {
    writeOAMdata(mem.buffer[address + i]);
  }
  cpu.cycles += 513;
  if (cpu.cycles % 2 != 0) cpu.cycles++;
}

void printNameTable()
{
  for (int i = 0; i < NAMETABLE_ROW; i++) {
    for (int j = 0; j < NAMETABLE_COL; j++) {
      printf("%02x ", ppu.nametable[i][j]);
    }
    printf("\n");
  }
  printf("\n\n\n\n\n\n\n\n\n");
}

void horizontalIncrement() {}
void verticalIncrement() {}

u16 ppu_getMirrorAddress(u16 address)
{
  if (address < 0x2000) return address;
  if (address >= 0x4000) address &= 0x3FFF;
  if (address >= 0x3F20)
    return address & 0x3F1F;  // mirror every 20 byte
  else if (address >= 0x3F00 && address < 0x3F20)
    return address;
  else if (address >= 0x3000 && address < 0x3F00)
    address &= 0x2EFF;
  u16 screenAddress = getScreenMirror(address);
  if (address < 0x3000 && getMirror() == HORIZONTAL_MIRROR) {
    if (screenAddress == TOP_LEFT_SCREEN ||
        screenAddress == BOTTOM_LEFT_SCREEN)  // return bottom left or top left
      return address;
    else if (screenAddress == TOP_RIGHT_SCREEN)
      return address & 0x23FF;  // return top left
    else if (screenAddress == BOTTOM_RIGHT_SCREEN)
      return address & 0x2BFF;  // return bottom left
    else
      assert(0);
  }
  else if (address < 0x3000 && getMirror() == VERTICAL_MIRROR) {
    if (screenAddress == TOP_LEFT_SCREEN || screenAddress == TOP_RIGHT_SCREEN)
      return address;
    else if (screenAddress == BOTTOM_LEFT_SCREEN)
      return address & 0x23FF;
    else if (screenAddress == BOTTOM_RIGHT_SCREEN)
      return address & 0x27FF;
    else
      assert(0);
  }
  else
    assert(0);
  return address;
}
u16 getScreenMirror(u16 address)
{
  if (address >= 0x2000 && address < 0x2400)
    return TOP_LEFT_SCREEN;
  else if (address >= 0x2400 && address < 0x2800)
    return TOP_RIGHT_SCREEN;
  else if (address >= 0x2800 && address < 0x2C00)
    return BOTTOM_LEFT_SCREEN;
  else
    return BOTTOM_RIGHT_SCREEN;
}
void ppu_tick(u16 esplased) {}

void fetchTile() {}
void fetchLowByte() {}

void fetchHighByte() {}
void generatePixel() {}
void spriteEvaluation() {}
void generateSpritePixel(Sprite* sprite, u8 row) {}
