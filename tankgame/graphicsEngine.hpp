#ifndef graphicsEngine_hpp
#define graphicsEngine_hpp

#define GL_SILENCE_DEPRECATION
#include <GLUT/GLUT.h>
#include "gameState.h"
#include <chrono>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include "physicsEngine.hpp"

void read_map();
void graphics_engine_init(int *argcp, char **argv);
void render();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void keyboard_up(unsigned char key, int x, int y);
void special_key(int key, int x, int y);
void special_key_up(int key, int x, int y);
void MouseMotion(int x,int y);

static int WINDOW_WIDTH=2000;
static int WINDOW_HEIGHT=2000;
static int GAME_WINDOW_LEN=500;
static double TANK_SPEED=0.4;
static double BULLET_SPEED=2;
static double SHIFT_WIDTH=0;
static double SHIFT_HEIGHT=0;
static double TANK_SIZE=0.06;
static double BULLET_SIZE=0.02;
static bool w_pressed=false;
static bool a_pressed=false;
static bool s_pressed=false;
static bool d_pressed=false;
static bool space_pressed=false;
static unsigned long last_frame_nano=0;
static unsigned long last_bullet_nano=0;
#endif /* graphicsEngine_hpp */
