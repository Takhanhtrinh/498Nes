#include "controller.h"
void createController() {
  for (int i = 0; i < 8; i++) {
    controller.buttons[i] = 0;
  }
  controller.strode = 0;
  controller.index = 0;
}
void controller_write(u8 value) {
  controller.strode = value;
  if ((controller.strode & 1) == 1) 
    controller.index = 0;
}
u8 controller_read() {
  u8 rt = 0;
  if (controller.index < 8 && controller.buttons[controller.index] != 0)
    rt = 1;
  controller.index++;
  if ((controller.strode & 0x01) == 0x01)
    controller.index = 0;
  
  return rt;
}
