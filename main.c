#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "controller.h"
#include "cpu.h"
#include "helper.h"
#include "ppu.h"
#include "rom.h"
time_t t0;
time_t t1;
CPU cpu;
PPU ppu;
MEM mem;
Controller controller;
u16 opcode_address = 0;
u8 opcode_value = 0;
u8 address_mode = 0;
INES ines;
u64 lastCycles = 0;

SDL_Window* screen;
SDL_Renderer* renderer;
SDL_Texture* texture;
void showRegisters(CPU* cpu, PPU* ppu)
{
  if (cpu == NULL || ppu == NULL) return;
  printf("pc: %04x ", cpu->pc);
  printf("A:%02x ", cpu->a);
  printf("X:%02x ", cpu->x);
  printf("Y:%02x ", cpu->y);
  printf("P:%02x ", cpu->p);
  printf("SP:%02x ", cpu->stack);
  printf("cycles: %u ", cpu->cycles);
  // printf("2000: %02x ", ppu.ppu_ctrl);
  // printf("2002: %02x ", ppu.ppu_status);
  printf("ppu cycles: %u ", ppu->cycles);
  printf("scanlines: %u ", ppu->scanlines);
}
void showStack(CPU* cpu)
{
  int stack = 0xff + STACK_OFFSET;

  for (int i = stack; i >= 0; i--) {
    printf("stack : %04x = %04x\n", i, mem.buffer[i]);
  }
}
void loadRoom(char* rom)
{
  int status = read_room(rom);
  if (status != 1) {
    printf("error while reading file\n");
    exit(1);
  }
  interrupt_reset();
  printf("room start at: %04x\n", cpu.pc);
  cpu.cycles = 7;
}
void updateTexture()
{
  SDL_UpdateTexture(texture, NULL, ppu.screen, SCREEN_WIDTH * sizeof(uint32_t));

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    printf("please enter rom name\n");
    return 0;
  }
  else {
    printf("room name: %s\n", argv[1]);
  }
  create_cpu();
  create_mem();
  create_ppu();
  createController();

  loadRoom(argv[1]);
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_CreateWindow("MyNES", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
  renderer = SDL_CreateRenderer(screen, -1, 0);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH,
                              SCREEN_HEIGHT);

  int quit = 0;

  SDL_Event event;
  u32 lastTime = 0, currentTime;
  while (!quit) {
    u32 currentFrame = ppu.frame;
    while (currentFrame == ppu.frame) {
      u8 numberOfCycles = emulate();
      if (numberOfCycles > 0)
        ppu_tick(numberOfCycles);
    }
    currentTime = SDL_GetTicks();
    lastTime = currentTime;
    usleep((16 - (currentTime - lastTime)) * 1000);

    while (SDL_PollEvent(&event)) switch (event.type) {
        case SDL_QUIT:
          quit = 1;
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_r:
              controller.buttons[BUTTON_SELECT] = 1;
              printf("select pressed\n");

              break;
            case SDLK_RETURN:
              controller.buttons[BUTTON_START] = 1;
              printf("return pressed \n");
              break;
            case SDLK_LEFT:
              controller.buttons[BUTTON_LEFT] = 1;
              break;
            case SDLK_RIGHT:
              controller.buttons[BUTTON_RIGHT] = 1;
              break;
            case SDLK_UP:
              controller.buttons[BUTTON_UP] = 1;
              break;
            case SDLK_DOWN:
              controller.buttons[BUTTON_DOWN] = 1;
              break;
            case SDLK_SPACE:
              controller.buttons[BUTTON_A] = 1;
              break;
          }
          break;
        case SDL_KEYUP:
          switch (event.key.keysym.sym) {
            case SDLK_r:
              controller.buttons[BUTTON_SELECT] = 0;
              break;

            case SDLK_RETURN:
              controller.buttons[BUTTON_START] = 0;
              break;
            case SDLK_LEFT:
              controller.buttons[BUTTON_LEFT] = 0;
              break;
            case SDLK_RIGHT:
              controller.buttons[BUTTON_RIGHT] = 0;
              break;
            case SDLK_UP:
              controller.buttons[BUTTON_UP] = 0;
              break;
            case SDLK_DOWN:
              controller.buttons[BUTTON_DOWN] = 0;
              break;
            case SDLK_SPACE:
              controller.buttons[BUTTON_A] = 0;
              break;
          }
          break;
      }
  }
  printf("quit\n");
  SDL_DestroyWindow(screen);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);
  SDL_Quit();

  return 0;
}
