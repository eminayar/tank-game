#include "gameState.h"
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctime>
#include <chrono>
#include <iostream>
#include <cstdlib> 
#include <unistd.h> 
#include <netdb.h> 
#include <map>

#define AD_PORT 12345
#define AD_RESP_PORT 12347
#define GAME_PORT 12346
#define PLAYER_PORT 12348
#define TANK_SIZE 0.06
#define BULLET_SIZE 0.02

using namespace std;

typedef pair <double,double> dd;
GameState *game_state=new GameState(),*game_state_buffer=new GameState();
mutex mtx;
bool pending_gamestate_change = false;
Map *game_map = new Map();
bool new_game_state=false;
map <string,int> ip_to_id;
map <string,int> :: iterator it;
vector <string> ip_adresses;
int id_counter = 0;
struct mycolor{
    double r,g,b;
    mycolor(double _r=0, double _g=0, double _b=0){
        r=_r;
        g=_g;
        b=_b;
    }
};
vector <dd> start_positions = {dd(-0.88,-0.91),dd(-0.88,0.91),dd(0.88,0.91),dd(0.88,-0.91)};
vector <mycolor> color_positions = {mycolor(0,0,255),mycolor(0,255,0),mycolor(255,0,0),mycolor(0,255,255)};

template <class Container>
void split(const std::string& str, Container& cont,
              const std::string& delims = ",")
{
    std::size_t current, previous = 0;
    current = str.find_first_of(delims);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find_first_of(delims, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

bool collision_check( Rectangle first , Rectangle second){
    double min_x=2,max_x=-2,min_y=2,max_y=-2;
    for(auto point:second.corners ){
        min_x=std::min(min_x,point.x);
        min_y=std::min(min_y,point.y);
        max_x=std::max(max_x,point.x);
        max_y=std::max(max_y,point.y);
    }
    for(auto point:first.corners ){
        if( point.x >= min_x && point.x <= max_x && point.y >= min_y && point.y <= max_y ){
            return false;
        }
    }
    return true;
}

Rectangle player_to_rect(double x, double y){
    Rectangle rect = Rectangle();
    double sz = TANK_SIZE+0.00;
    rect.corners.push_back(Point(x-sz, y-sz));
    rect.corners.push_back(Point(x-sz, y+sz));
    rect.corners.push_back(Point(x+sz, y+sz));
    rect.corners.push_back(Point(x+sz, y-sz));
    return rect;
}

Rectangle bullet_to_rect(double x, double y){
    Rectangle rect = Rectangle();
    double sz = BULLET_SIZE+0.00;
    rect.corners.push_back(Point(x-sz, y-sz));
    rect.corners.push_back(Point(x-sz, y+sz));
    rect.corners.push_back(Point(x+sz, y+sz));
    rect.corners.push_back(Point(x+sz, y-sz));
    return rect;
}

bool check_validity_of_bullet(double x, double y, int owner){
    Rectangle bullet_as_rect = bullet_to_rect(x,y);
    for( auto object:game_map->objects ){
        if( !collision_check(bullet_as_rect, object) ){
            return false;
        }
    }
    if( x-BULLET_SIZE < -1 || x+BULLET_SIZE > 1 || y-BULLET_SIZE < -1 || y+BULLET_SIZE > 1 )
        return false;
    for(auto &player:game_state->players){
        if(player.player_id == owner)
            continue;
        Rectangle player_as_rect = player_to_rect(player.ref.x, player.ref.y);
        if( !collision_check(bullet_as_rect, player_as_rect) ){
            player.ref.x = start_positions[player.player_id].first;
            player.ref.y = start_positions[player.player_id].second;
            return false;
        }
    }
    return true;
}

inline void collision_detection(){
    for(auto player:game_state->players){
        for( int i=0 ; i<(int)player.bullets.size() ; i++ ){
            if( !check_validity_of_bullet(player.bullets[i].ref.x, player.bullets[i].ref.y, player.player_id)){
                player.bullets.erase(player.bullets.begin()+i);
                i--;
            }
        }
    }
}

int game_state_announcer(){
    int sock;
    int broadcast = 1;
    struct sockaddr_in broadcast_addr;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    int count;
    int ret;
    fd_set readfd;
    char buffer[1024];
    int i;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("sock error");
        return -1;
    }
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(broadcast));
    if (ret == -1) {
        perror("setsockopt error");
        return 0;
    }
    int len = sizeof(struct sockaddr_in);

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(GAME_PORT);
    while(true){
        collision_detection();
        string msg = to_string(game_state->players.size());
        for(auto player:game_state->players){
            msg+= ","+to_string(player.ref.x)+","+to_string(player.ref.y)+","+to_string(player.direction);
            msg+=","+to_string(player.bullets.size());
            for(auto bullet:player.bullets){
                msg+=","+to_string(bullet.ref.x)+","+to_string(bullet.ref.y)+","+to_string(bullet.direction);
            }
        }
        char message[1024];
        strcpy(message, msg.c_str());
        ret = sendto(sock, message, strlen(message), 0, (struct sockaddr*) &broadcast_addr, sizeof(broadcast_addr));
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

int advertiser(string my_ip){
    cout << "advertiser started" << " " << my_ip << endl;
    int sock;
    int broadcast = 1;
    struct sockaddr_in broadcast_addr;
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    int count;
    int ret;
    fd_set readfd;
    char buffer[1024];
    int i;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("sock error");
        return -1;
    }
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast, sizeof(broadcast));
    if (ret == -1) {
        perror("setsockopt error");
        return 0;
    }

    int len = sizeof(struct sockaddr_in);

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(AD_PORT);
    while(true){
        string msg = my_ip;
        for(auto adress:ip_adresses){
            msg +=  ","+adress;
        }
        char message[1024];
        strcpy(message, msg.c_str());
        ret = sendto(sock, message, strlen(message), 0, (struct sockaddr*) &broadcast_addr, sizeof(broadcast_addr));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int direct_message_listener(){
    cout << "dm listener started" << endl;
    while(true){
        int sock;
        int yes = 1;
        struct sockaddr_in client_addr;
        struct sockaddr_in server_addr;
        socklen_t addr_len;
        int count;
        int ret;
        fd_set readfd;
        char buffer[1024];

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("sock error\n");
            return -1;
        }

        addr_len = sizeof(struct sockaddr_in);

        // memset((void*)&server_addr, 0, addr_len);
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(AD_RESP_PORT);

        ret = bind(sock, (const struct sockaddr*)&server_addr, addr_len);
        if (ret < 0) {
            perror("bind error\n");
            return -1;
        }
        while (1) {
            FD_ZERO(&readfd);
            FD_SET(sock, &readfd);

            ret = select(sock+1, &readfd, NULL, NULL, 0);
            if (ret > 0) {
                if (FD_ISSET(sock, &readfd)) {
                    count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&client_addr, &addr_len);
                    string s="";
                    for(int i=0; i<1024;i++ ){
                        if(!buffer[i])
                            break;
                        s+= buffer[i];
                    }
                    if( ip_to_id.find(s) == ip_to_id.end() ){
                        ip_to_id[s] = id_counter++;
                        ip_adresses.push_back(s);
                    }
                    Player new_player = Player();
                    new_player.player_id=id_counter-1;
                    new_player.ref.x=start_positions[id_counter-1].first;
                    new_player.ref.y=start_positions[id_counter-1].second;
                    new_player.direction = 0;
                    new_player.is_alive = true;
                    new_player.red = color_positions[id_counter-1].r;
                    new_player.green = color_positions[id_counter-1].g;
                    new_player.blue = color_positions[id_counter-1].b;
                    game_state->players.push_back(new_player);
                }
            }
        }
    }
}

int player_move_listener(){
    cout << "player move listener started" << endl;
    while(true){
        int sock;
        int yes = 1;
        struct sockaddr_in client_addr;
        struct sockaddr_in server_addr;
        socklen_t addr_len;
        int count;
        int ret;
        fd_set readfd;
        char buffer[1024];

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            perror("sock error\n");
            return -1;
        }

        addr_len = sizeof(struct sockaddr_in);

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(PLAYER_PORT);

        ret = bind(sock, (const struct sockaddr*)&server_addr, addr_len);
        if (ret < 0) {
            perror("bind error\n");
            return -1;
        }
        while(game_state->players.size() != 2 );
        while (1) {
            FD_ZERO(&readfd);
            FD_SET(sock, &readfd);

            ret = select(sock+1, &readfd, NULL, NULL, 0);
            if (ret > 0) {
                if (FD_ISSET(sock, &readfd)) {
                    count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&client_addr, &addr_len);
                    string s="";
                    for(int i=0; i<1024;i++ ){
                        if(!buffer[i])
                            break;
                        s+= buffer[i];
                    }
                    vector <string> data;
                    split(s,data);
                    int player_id = ip_to_id[data[0]];
                    Player &current_player = game_state->players[player_id];
                    current_player.ref.x  = stod(data[1]);
                    current_player.ref.y = stod(data[2]);
                    current_player.direction = stod(data[3]);
                    int M = stod(data[4]);
                    int start_index = 5;
                    current_player.bullets.clear();
                    for( int i=0 ; i<M ; i++ ){
                        Bullet new_bullet = Bullet();
                        new_bullet.ref.x = stod(data[start_index]);
                        new_bullet.ref.y = stod(data[start_index+1]);
                        new_bullet.direction = stod(data[start_index+2]);
                        current_player.bullets.push_back(new_bullet);
                        start_index += 3;
                    }
                }
            }
        }
    }
}

int main(int argc, char ** argv){
    thread ad_obj(advertiser, argv[1]);
    thread listener_obj(direct_message_listener);
    thread wagawg(game_state_announcer);
    thread player_move_obj(player_move_listener);
    while(true){
        string input;
        cin >> input;
        if(input == "exit"){
            exit(0);
        }
    }
    return 0;
}