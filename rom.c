#include "rom.h"
int read_room(const char* filePath) {
  FILE* fp;
  if ((fp = fopen(filePath, "r")) == NULL) {
    printf("read room failed, the file doesn't exist\n");
    return -1;
  } else {
    // read header
    fread((void*)&ines.fileFormat, 4, 1, fp);
    fread((void*)&ines.size_prg, 1, 1, fp);
    fread((void*)&ines.size_chr, 1, 1, fp);
    fread((void*)&ines.byte_6, 1, 1, fp);
    fread((void*)&ines.byte_7, 1, 1, fp);
    fread((void*)&ines.byte_8, 1, 1, fp);
    fseek(fp, 16, SEEK_SET);
      printf("mapper: %hhu\n", getMapper());
      printf("getTrainer %hhu\n", getTrainer());
      printf("size_prg: %hhu\n", ines.size_prg);
    if (getTrainer() != 0) {
      u8* trainerAddress = &mem.buffer[0x7000];
      fread((void*)trainerAddress, 0x200, 1, fp);
    }
    printf("room offset: %u\n", PRG_ADDRESS + (ines.size_prg % 2) * 0x4000);
    u8* room_address = &mem.buffer[PRG_ADDRESS + (ines.size_prg % 2) * 0x4000];

    fread((void*)room_address, 0x4000 * ines.size_prg, 1, fp);
    if (ines.size_chr != 0) {
      fread((void*)ppu.buffer, 0x2000 * ines.size_chr, 1, fp); 
    } 
    else {
      printf("the rom is using chr-ram\n");
    }
    u32 pointer = ftell(fp);
    printf("pointer: %u\n", pointer);
    fclose(fp);
    return 1;
  }
}
