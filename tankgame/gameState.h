#ifndef gameState_h
#define gameState_h

//#define MULTIPLAYER_DEATHMATCH
#include <vector>

struct PlayerInfo{
    double reference_point,direction;
    bool is_alive;
};

struct Bullets{
    double reference_point,direction,speed;
};

struct GameState{
    int game_mode;
    int num_players;
    std::vector <PlayerInfo> players;
    std::vector <Bullets> bullets;
};

GameState *game_state,*game_state_buffer;
bool pending_gamestate_change = false;
#endif /* gameState_h */
