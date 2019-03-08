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

void horizontalIncrement()
{
  // wrap around when it hits 31
  if ((ppu.ppu_address & 0x1F) == 0x1F) {
    // reset x increment
    ppu.ppu_address &= ~(0x1F);
    ppu.ppu_address ^= 0x0400;
  }
  else
    ppu.ppu_address++;
}
void verticalIncrement()
{
  if ((ppu.ppu_address & 0x7000) != 0x7000)
    ppu.ppu_address += 0x1000;
  else {
    ppu.ppu_address &= ~0x7000;
    int y = (ppu.ppu_address & 0x03E0) >> 5;
    if (y == 29) {
      y = 0;
      ppu.ppu_address ^= 0x0800;
    }
    else if (y == 31)
      y = 0;
    else
      y += 1;
    ppu.ppu_address = (ppu.ppu_address & ~0x03E0) | (y << 5);
  }
}

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
void ppu_tick(u16 esplased)
{
  // ppu runs 3 times faster than cpu
  u8 numberOfCycles = esplased * 3;
  ppu.cycles = (cpu.cycles * 3 - numberOfCycles) % 341;
  ppu.scanlines = ((cpu.cycles * 3 - numberOfCycles) / 341) % 262;
  for (int i = 0; i < numberOfCycles; i++) {
    u8 state = getScanlineState();
    if (getShowBackground() != 0) {
      u8 timeToFetchTile = (state == RENDER || state == PRE_RENDER);
      u8 isDisplay = (ppu.cycles < 257);
      u8 isPrefetch = (ppu.cycles > 320 && ppu.cycles < 337);
      if (timeToFetchTile && (isDisplay || isPrefetch)) fetchTile();
    }
    if (getShowSprite() != 0 && state == RENDER) {
      if (ppu.cycles == 320) spriteEvaluation();
    }
    updateRegisters(state);
  }
}
void updateRegisters(u8 state)
{
  switch (state) {
    case PRE_RENDER:
      // clear NMI, Sprite overflow and Sprite 0 hit
      if (ppu.cycles == 1) ppu.ppu_status &= 0x1F;
      // OAMADDR is set to 0 during each of ticks 257-320 (the sprite tile
      // loading interval) of the pre-render and visible scanlines.
      if (ppu.cycles >= 257 && ppu.cycles <= 320) ppu.oam_addr = 0;

      if (getShowBackground() != 0) {
        if (ppu.cycles % 8 == 0 && ppu.cycles != 0 &&
            (ppu.cycles < 256 || (ppu.cycles > 327 && ppu.cycles < 337)))
          horizontalIncrement();
        else if (ppu.cycles == 256)
          verticalIncrement();
        else if (ppu.cycles == 257)
          ppu.ppu_address =
              (ppu.ppu_address & 0xFBE0) | (ppu.temp_address & 0x041F);
        else if (ppu.cycles >= 280 && ppu.cycles <= 304 != 0) {
          ppu.ppu_address =
              (ppu.ppu_address & 0x841F) | (ppu.temp_address & 0x7BE0);
        }
      }
      break;

    case RENDER:
      // OAMADDR is set to 0 during each of ticks 257-320 (the sprite tile
      // loading interval) of the pre-render and visible scanlines.
      if (ppu.cycles >= 257 && ppu.cycles <= 320) ppu.oam_addr = 0;
      // https://wiki.nesdev.com/w/images/d/d1/Ntsc_timing.png
      if (getShowBackground() != 0) {
        if (ppu.cycles % 8 == 0 && ppu.cycles != 0 &&
            (ppu.cycles < 256 || (ppu.cycles > 327 && ppu.cycles < 337)))
          horizontalIncrement();
        else if (ppu.cycles == 256)
          verticalIncrement();
        else if (ppu.cycles == 257)
          ppu.ppu_address =
              (ppu.ppu_address & 0xFBE0) | (ppu.temp_address & 0x041F);
      }
      break;

    case POST_RENDER:
      if (ppu.cycles == 0) {
        if (getShowBackground() != 0) {
             updateTexture();
            ppu.frame++;
        }
      }
      break;

    case VBLANK:
      // // Vertical blank has started (0: not in vblank; 1: in vblank).
      // // Set at dot 1 of line 241 (the line *after* the post-render
      // // line); cleared after reading $2002 and at dot 1 of the
      // // pre-render line,
      // https://wiki.nesdev.com/w/index.php/PPU_programmer_reference#Status_.28.242002.29_.3C_read

      if (ppu.cycles == 1 && ppu.scanlines == 241) {
        ppu.ppu_status |= VBLANK_OCCUR;

        if (getNMI() != 0) cpu.nmi_interrupt = 1;
      }
      break;
  }
  ppu.cycles++;
  if (ppu.cycles >= CLOCK_PER_SCANLINE) {
    ppu.cycles %= CLOCK_PER_SCANLINE;
    ppu.scanlines++;
    if (ppu.scanlines >= TOTAL_SCANLINE) ppu.scanlines %= TOTAL_SCANLINE;
  }
}

void fetchTile()
{
  switch (ppu.cycles % 8) {
    case 0:
      // if (ppu.cycles < 257) {
      //   // copy prefetch to nametable
      //   if (ppu.cycles <= 8 && getScanlineState() == RENDER)
      //     ppu.nametable[y][ppu.cycles / 8] =
      //         ppu.prefetchNameTable[ppu.cycles / 8];
      //   else if (ppu.cycles > 8 && getScanlineState() == RENDER)
      //     ppu.nametable[y][x] = ppu.nametableByte;
      //   // if (getShowBackground() != 0) printNameTable();
      // }
      generatePixel();
      break;

    case 1: {
      u16 addr = fetchNameTable();
      addr = ppu_getMirrorAddress(addr);
      ppu.nametableByte = ppu.buffer[addr];
    } break;
    case 3: {
      u16 addr = fetchAttribute();
      addr = ppu_getMirrorAddress(addr);
      ppu.attributeByte = ppu.buffer[addr];
      // https://wiki.nesdev.com/w/index.php/PPU_attribute_tables
      // change for 2 bytes for x increment and y increment of ppu_address
      u8 numberBitShift =
          (ppu.attributeByte & 0x02) | ((ppu.attributeByte & 0x40) >> 4);
      ppu.attributeByte = (ppu.attributeByte >> numberBitShift) & 0x03;
    } break;
    case 5:
      fetchLowByte();
      break;
    case 7:
      fetchHighByte();
      break;
  }
}
void fetchLowByte()
{
  u16 nametable = (u16)ppu.nametableByte;
  // what row does the pixels fall to, get the fine scroll y
  u8 row = (ppu.ppu_address >> 12) & 7;
  u16 patternAddress = getBackgroundPatternTableAddress();
  patternAddress = (patternAddress | (nametable << 4)) + row;
  ppu.lowByte = ppu.buffer[patternAddress];
}

void fetchHighByte()
{
  u16 nametable = (u16)ppu.nametableByte;
  // what row does the pixels fall to, get the fine scroll y
  u8 row = (ppu.ppu_address >> 12) & 7;
  u16 patternAddress = getBackgroundPatternTableAddress();
  patternAddress = (patternAddress | (nametable << 4)) + row;
  ppu.highByte = ppu.buffer[patternAddress + 8];
}
void generatePixel()
{
  u16 x;
  if (ppu.cycles == 0) return;
  if (ppu.cycles < 257)
    x = ppu.cycles - 1;
  else if (ppu.cycles > 320 && ppu.cycles < 337)
    // 0 or 1(pre fetch for next scanline)
    x = ppu.cycles / 336;
  else
    assert(0);
  u16 y = 0;
  if (ppu.scanlines < 240) y = ppu.scanlines;
  u8 attribute = ppu.attributeByte;
  for (int i = 0; i < 8; i++, x++) {
    if (x > 255) return;
    u8 bit0 = (ppu.lowByte & 0x80) >> 7;
    u8 bit1 = (ppu.highByte & 0x80) >> 6;
    // shift 1 bit for both highByte and lowByte
    ppu.lowByte <<= 1;
    ppu.highByte <<= 1;
    u16 colorAddress = getBGcolorAddress(attribute) + (bit1 | bit0);
    u8 color = ppu.buffer[colorAddress];
    u32 colorValue = Pattle[color];
    ppu.screen[y * SCREEN_WIDTH + x] = colorValue;
  }
}
void spriteEvaluation()
{
  if (ppu.scanlines == 0) return;
  u8 y = ppu.scanlines;

  u8 spriteCount = 0;
  for (int i = 0; i < 256; i += 4) {
    if (spriteCount >= 8) break;
    u8 spriteY = ppu.oam_buffer[i];
    // out of range 
    u8 index = ppu.oam_buffer[i + 1];
    u8 attribute = ppu.oam_buffer[i + 2];
    u8 spriteX = ppu.oam_buffer[i + 3];
    if (i == 0 && spriteX != 0) ppu.ppu_status |= SPRITE_HIT;

    u8 spriteBottomY = 0;
    // check if the sprite is in range
    if (getSpriteSize() == SPRITE_8X8)
      spriteBottomY = spriteY + 8;
    else
      spriteBottomY = spriteY + 16;
    // check in range
    if (y >= spriteY && y < spriteBottomY) {
      // fetch sprite and render
      Sprite s;
      s.y = spriteY;
      s.x = spriteX;
      s.attribute = attribute;
      s.tile = index;
      u8 row = y - spriteY;
      generateSpritePixel(&s, row);
      spriteCount++;
    }
    else
      continue;
  }
}
void generateSpritePixel(Sprite* sprite, u8 row)
{
  if (sprite == NULL) return;
  u16 y = ppu.scanlines;
  if (getSpriteSize() == SPRITE_8X8) {
    u16 patternAddress =
        (getSpritePatternTableAddress() | (((u16)sprite->tile) << 4)) + row;
    u8 lowByte = ppu.buffer[patternAddress];
    u8 highByte = ppu.buffer[patternAddress + 8];
    for (int i = 0; i < 8; i++) {
      u8 bit0 = (lowByte & 0x80) >> 7;
      u8 bit1 = (highByte & 0x80) >> 6;
      lowByte <<= 1;
      highByte <<= 1;
      if ((bit1 | bit0) == 0) continue;
      u16 colorAddress =
          getSpriteColorAddress(sprite->attribute & 0x03, bit1 | bit0);
      u8 pattle = ppu.buffer[colorAddress];
      u32 color = Pattle[pattle];
      u16 offset = y * SCREEN_WIDTH + (u16)sprite->x + i;
      ppu.screen[offset] = color;
    }
  }
  else {
    assert(0);
  }
}
