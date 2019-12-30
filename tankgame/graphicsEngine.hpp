#ifndef graphicsEngine_hpp
#define graphicsEngine_hpp

#define GL_SILENCE_DEPRECATION
#define AD_PORT 12345
#define AD_RESP_PORT 12347
#define GAME_PORT 12346
#define PLAYER_PORT 12348
#include <GLUT/GLUT.h>
#include "gameState.h"
#include <chrono>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctime>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <netdb.h>
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
static unsigned long last_state_nano=0;
static unsigned long last_bullet_nano=0;
extern Player me;
extern bool new_data_available;
extern std::string server_ip;
extern std::string my_ip;

struct mycolor{
    double r,g,b;
    mycolor(double _r=0, double _g=0, double _b=0){
        r=_r;
        g=_g;
        b=_b;
    }
};
#endif /* graphicsEngine_hpp */
