#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "definition.h" 
#include <stdlib.h>
#include <stdio.h>
#define BUTTON_A 0 
#define BUTTON_B 1
#define BUTTON_SELECT 2
#define BUTTON_START 3 
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7
typedef struct {
  u8 buttons[8];
  u8 strode;
  u8 index;
} Controller;
extern Controller controller;
void createController();
void controller_write(u8 value);
u8 controller_read();
#endif
