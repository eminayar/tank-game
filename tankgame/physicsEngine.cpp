#include "physicsEngine.hpp"

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
            std::cout << min_x << " " << min_y << " " << max_x << " " << max_y << std::endl;
            std::cout << point.x << " " << point.y << std::endl;
            return false;
        }
    }
    return true;
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

Rectangle player_to_rect(double x, double y){
    Rectangle rect = Rectangle();
    double sz = TANK_SIZE+0.00;
    rect.corners.push_back(Point(x-sz, y-sz));
    rect.corners.push_back(Point(x-sz, y+sz));
    rect.corners.push_back(Point(x+sz, y+sz));
    rect.corners.push_back(Point(x+sz, y-sz));
    return rect;
}

bool check_validity_of_bullet(double x, double y){
    Rectangle bullet_as_rect = bullet_to_rect(x,y);
    for( auto object:game_map->objects ){ //check collusion with walls
        if( !collision_check(bullet_as_rect, object) ){
            return false;
        }
    }
    if( x-BULLET_SIZE < -1 || x+BULLET_SIZE > 1 || y-BULLET_SIZE < -1 || y+BULLET_SIZE > 1 )
        return false;
    return true;
}

bool check_validity_of_player(double x, double y){
    Rectangle player_as_rect = player_to_rect(x, y);
    for( auto object:game_map->objects ){ //check collusion with walls
        if( !collision_check(player_as_rect, object) ){
            return false;
        }
    }
    if( x-TANK_SIZE < -1 || x+TANK_SIZE > 1 || y-TANK_SIZE < -1 || y+TANK_SIZE > 1 )
        return false;
    return true;
}
