#include "mem.h"
void create_mem() {
  for (int i = 0; i < MEM_SIZE; i++) {
    mem.buffer[i] = 0x00;
  }
}

void write_mem_w(u16 value, u16 address) {
  mem.buffer[address] = (u8)(value & 0xff);
  mem.buffer[address + 1] = (u8)((value >> 8) & 0xff);
}
u16 read_mem_w(u16 address) {
  u16 byte1 = mem.buffer[address];
  u16 byte2 = mem.buffer[address + 1];
  return (byte2 << 8) | byte1;
}
void write_mem_b(u8 value, u16 address) {
  getMirrorAddress(&address);
  mem.buffer[address] = value;
  if (address == PPU_R_CONTROL_ADDRESS) {
    writePPUCtr(value);
#ifdef PPUDEBUG
    printf("write to ppuctrl $2000: %02x\n", ppu.ppu_ctrl);
#endif
  } else if (address == PPU_R_MASK_ADDRESS) {
    writePPUMask(value);
#ifdef PPUDEBUG
    printf("write to ppumask $2001: %02x\n", ppu.ppu_mask);
#endif
  } else if (address == PPU_R_OAM_ADDRESS) {
    writeOAMaddress(value);
#ifdef PPUDEBUG
    printf("write to OAMADDR $2003: %02x\n", ppu.oam_addr);
#endif
  } else if (address == PPU_R_OAM_DATA_ADDRESS) {
    writeOAMdata(value);
#ifdef PPUDEBUG
    printf("write to OAMDATA $2004: %02x\n", ppu.oam_dma);
#endif
  } else if (address == PPU_R_SCROLL_ADDRESS) {
    writePPUscroll(value);
#ifdef PPUDEBUG
    printf("write to PPUSCROLL $2005: %02x\n", ppu.ppu_scroll);
#endif
  } else if (address == PPU_R_ADDRESS_ADDRESS) {
    writePPUaddr(value);
#ifdef PPUDEBUG
    printf("write to PPUADDR $2006: %02x\n", ppu.ppu_address);
#endif
  } else if (address == PPU_R_DATA_ADDRESS) {
    writePPUdata(value);
#ifdef PPUDEBUG
    printf("write to PPUDATA $2007 address: %04x =  %02x\n", ppu.ppu_address,
           ppu.buffer[ppu.ppu_address - getAddressIncrement()]);
#endif
  } else if (address == PPU_R_OAM_DMA_ADDRESS) {
    writeDMA(value);
#ifdef PPUDEBUG
    printf("write to OAMDMA $4014:\n");
#endif
  } else if (address == 0x4016) {
    controller_write(value);
  }
}

u8 read_mem_b(u16 address) {
  getMirrorAddress(&address);
  if (address == PPU_R_STATUS_ADDRESS) {
    mem.buffer[address] = readPPUstatus();
#ifdef PPUDEBUG
    printf("read from PPUSTATUS $2002: %02x\n", mem.buffer[address]);
#endif
  } else if (address == PPU_R_DATA_ADDRESS) {
    mem.buffer[address] = readPPUdata();
#ifdef PPUDEBUG
    printf("read from PPUDATA $2007: %02x\n", mem.buffer[address]);
#endif

  } else if (address == PPU_R_OAM_DATA_ADDRESS) {
    mem.buffer[address] = readOAMdata();
#ifdef PPUDEBUG
    printf("read from PPUDATA $2004: %02x\n", mem.buffer[address]);
#endif

  } else if (address == 0x4016 ) {
    mem.buffer[address] = controller_read();
  }
  return mem.buffer[address];
}
void getMirrorAddress(u16* address) {
  if (*address < 0x2000)
    *address = *address & 0x07FF;
  else if (*address < 0x4000)
    *address = *address & 0x2007;
}
