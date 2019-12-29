#ifndef gameState_h
#define gameState_h

//#define MULTIPLAYER_DEATHMATCH
#include <vector>
#include <string>

struct Point{
    double x,y;
    Point(double _x=0,double _y=0){
        x=_x;
        y=_y;
    }
};

struct Bullet{
    Point ref;
    double direction;
};


struct Player{
    double direction;
    Point ref;
    bool is_alive;
    std::string player_id;
    double red,geen,blue;
    std::vector <Bullet> bullets;
};

struct Rectangle{
    std::vector <Point> corners;
    double red,green,blue;
};

struct GameState{
    //state
    int num_players;
    std::vector <Player> players;
};

struct Map{
    int num_objects;
    std::vector <Rectangle> objects;
};

extern GameState *game_state,*game_state_buffer;
extern bool pending_gamestate_change;
extern Map *game_map;
#endif /* gameState_h */
