//
//  main.cpp
//  tankgame
//
//  Created by Emin Ayar on 29.12.2019.
//  Copyright © 2019 Emin Ayar. All rights reserved.
//

#include "graphicsEngine.hpp"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <string.h>

bool found_server = false;
bool found_my_ip = false;
std::string server_ip;
std::string my_ip;
std::vector <mycolor> color_positions = {mycolor(0,0,255),mycolor(0,255,0),mycolor(255,0,0),mycolor(0,255,255)};

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

int announcement_listener(){
    int sock;
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

    addr_len = (long)sizeof(struct sockaddr_in);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(AD_PORT);

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
                std::string s="";
                for(int i=0; i<1024;i++ ){
                    if(!buffer[i])
                        break;
                    s+= buffer[i];
                }
                std::cout << s << std::endl;
                std::vector <std::string> data;
                split(s,data);
                server_ip = data[0];
                found_server=true;
                for(int i=1 ; i<data.size() ; i++ ){
                    std::string ip_addr = data[i];
                    if(ip_addr == my_ip){
                        found_my_ip = true;
                        me.player_id = i-1;
                        return 0;
                    }
                }
                char message[1024];
                strcpy(message, my_ip.c_str());
                int sockfd;
                struct sockaddr_in servaddr;
                bzero(&servaddr, sizeof(servaddr));
                servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());
                servaddr.sin_port = htons(AD_RESP_PORT);
                servaddr.sin_family = AF_INET;
                sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
                {
                    printf("\n Error : Connect Failed \n");
                    return -1;
                }
                sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)NULL, sizeof(servaddr));
                close(sockfd);
            }
        }
    }
}

int server_listener(){
    int sock;
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

    addr_len = (long)sizeof(struct sockaddr_in);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(GAME_PORT);

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
            if (FD_ISSET(sock, &readfd) && new_data_available) {
                count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&client_addr, &addr_len);
                std::string s="";
                for(int i=0; i<1024;i++ ){
                    if(!buffer[i])
                        break;
                    s+= buffer[i];
                }
                std::cout << s << std::endl;
                std::vector <std::string> data;
                split(s,data);
                game_state_buffer->players.clear();
                int N = stoi(data[0]);
                int indexer = 1;
                for( int i=0 ; i<N ; i++ ){
                    Player p = Player();
                    p.ref.x = stod(data[indexer]);
                    p.player_id=i;
                    p.ref.y = stod(data[indexer+1]);
                    p.direction = stod(data[indexer+2]);
                    p.red=color_positions[i].r;
                    p.green=color_positions[i].g;
                    p.blue=color_positions[i].b;
                    int M = stoi(data[indexer+3]);
                    for( int j=0 ; j<M ; j++ ){
                        Bullet bullet = Bullet();
                        bullet.ref.x = stod(data[indexer+4+j*3]);
                        bullet.ref.y = stod(data[indexer+5+j*3]);
                        bullet.direction = stod(data[indexer+6+j*3]);
                        bullet.is_alive = true;
                        p.bullets.push_back(bullet);
                    }
                    indexer+=M*3+4;
                    game_state_buffer->players.push_back(p);
                }
                new_data_available = true;
            }
        }
    }
}


int main(int argc, char ** argv){
    my_ip = argv[1];
    std::thread network_listener(announcement_listener);
    std::thread game_listener(server_listener);
    graphics_engine_init(&argc,argv);
    return 0;
}

