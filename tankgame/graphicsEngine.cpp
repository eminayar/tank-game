//
//  graphicsEngine.cpp
//  tankgame
//
//  Created by Emin Ayar on 29.12.2019.
//  Copyright © 2019 Emin Ayar. All rights reserved.
//
#include "graphicsEngine.hpp"

GameState *game_state,*game_state_buffer;
bool pending_gamestate_change = false;
Map *game_map;
Player me = Player();

void graphics_engine_init(int *argcp, char **argv){
    game_map = new Map();
    game_state = new GameState();
    game_state_buffer = new GameState();
    read_map();
    glutInit(argcp, argv);
    me.ref.x=0;
    me.ref.y=-0.2;
    me.direction=0.0;
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(2000,2000);
    glutCreateWindow("TANK");
    glClearColor (255, 255, 255, 0.0);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glutDisplayFunc(render);
    glutIdleFunc(render);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboard_up);
    glutSpecialFunc(special_key);
    glutSpecialUpFunc(special_key_up);
    glutPassiveMotionFunc(MouseMotion);
    glutMainLoop();
}

inline void render_map(){
    glBegin(GL_QUADS);
    glColor3d((GLdouble) 0, (GLdouble) 0, (GLdouble) 0 );
    glVertex2f(0,0);
    glVertex2f(0,WINDOW_HEIGHT);
    glVertex2f(SHIFT_WIDTH,WINDOW_HEIGHT);
    glVertex2f(SHIFT_WIDTH,0);
    glEnd();
    glBegin(GL_QUADS);
    glColor3d((GLdouble) 0, (GLdouble) 0, (GLdouble) 0 );
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN,0);
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN,WINDOW_HEIGHT);
    glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT);
    glVertex2f(WINDOW_WIDTH,0);
    glEnd();
    glBegin(GL_QUADS);
    glColor3d((GLdouble) 0, (GLdouble) 0, (GLdouble) 0 );
    glVertex2f(0,0);
    glVertex2f(0,SHIFT_HEIGHT);
    glVertex2f(WINDOW_WIDTH,SHIFT_HEIGHT);
    glVertex2f(WINDOW_WIDTH,0);
    glEnd();
    glBegin(GL_QUADS);
    glColor3d((GLdouble) 0, (GLdouble) 0, (GLdouble) 0 );
    glVertex2f(0,SHIFT_HEIGHT+GAME_WINDOW_LEN);
    glVertex2f(0,WINDOW_HEIGHT);
    glVertex2f(WINDOW_WIDTH,WINDOW_HEIGHT);
    glVertex2f(WINDOW_WIDTH,SHIFT_HEIGHT+GAME_WINDOW_LEN);
    glEnd();
    for(auto object:game_map->objects){
        glBegin(GL_QUADS);
        glColor3d(object.red,object.green,object.blue);
        for(auto point:object.corners){
            double render_x = SHIFT_WIDTH+GAME_WINDOW_LEN*((point.x+1.0)/2.0);
            double render_y = SHIFT_HEIGHT+GAME_WINDOW_LEN*((point.y+1.0)/2.0);
            glVertex2f(render_x,render_y);
        }
        glEnd();
    }
}

inline void drawSquare(Point p, double size, double r , double g, double b){
    glBegin(GL_QUADS);
    glColor3d((GLdouble) r, (GLdouble) g, (GLdouble) b );
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN*((p.x+1.0-size)/2.0),SHIFT_HEIGHT+GAME_WINDOW_LEN*((p.y+1.0-size)/2.0));
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN*((p.x+1.0-size)/2.0),SHIFT_HEIGHT+GAME_WINDOW_LEN*((p.y+1.0+size)/2.0));
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN*((p.x+1.0+size)/2.0),SHIFT_HEIGHT+GAME_WINDOW_LEN*((p.y+1.0+size)/2.0));
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN*((p.x+1.0+size)/2.0),SHIFT_HEIGHT+GAME_WINDOW_LEN*((p.y+1.0-size)/2.0));
    glEnd();
}

inline void drawTank(Player p){
    drawSquare(p.ref, TANK_SIZE, 0, 0, 255 );
    drawSquare(p.ref, TANK_SIZE/2.0, 0, 0, 0 );
    glBegin(GL_LINES);
    glColor3d((GLdouble) 0, (GLdouble) 0, (GLdouble) 0 );
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN*((p.ref.x+1.0)/2.0), SHIFT_HEIGHT+GAME_WINDOW_LEN*((p.ref.y+1.0)/2.0));
    glVertex2f(SHIFT_WIDTH+GAME_WINDOW_LEN*((p.ref.x+1.0+cos(p.direction)*TANK_SIZE)/2.0), SHIFT_HEIGHT+GAME_WINDOW_LEN*((p.ref.y+1.0+sin(p.direction)*TANK_SIZE)/2.0));
    glEnd();
    for(auto bullet:p.bullets){
        drawSquare(bullet.ref, BULLET_SIZE, 0, 0, 255);
    }
}

void render(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    auto now=std::chrono::high_resolution_clock::now();
    unsigned long nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    
    render_map();
    drawTank(me);
    
    double new_x=me.ref.x,new_y=me.ref.y;
    
    if(last_frame_nano == 0){
        last_frame_nano = nanos;
        glutSwapBuffers();
        return;
    }
    
    double diff = ((double)(nanos-last_frame_nano))/1e9;
    
    if( w_pressed ){
        new_y = me.ref.y-diff*TANK_SPEED;
    }else if( s_pressed ){
        new_y = me.ref.y+diff*TANK_SPEED;
    }
    if( a_pressed ){
        new_x = me.ref.x-diff*TANK_SPEED;
    }else if ( d_pressed ){
        new_x = me.ref.x+diff*TANK_SPEED;
    }
    if( check_validity_of_player(new_x, new_y)){
        me.ref.x = new_x;
        me.ref.y = new_y;
    }
    
    for( int i=0 ; i<(int)me.bullets.size() ; i++ ){
        Bullet &bullet = me.bullets[i];
        new_x=bullet.ref.x + cos(bullet.direction)*BULLET_SPEED*diff;
        new_y=bullet.ref.y + sin(bullet.direction)*BULLET_SPEED*diff;
        if( check_validity_of_bullet(new_x, new_y) ){
            bullet.ref.x = new_x;
            bullet.ref.y = new_y;
        }else{
            me.bullets.erase(me.bullets.begin()+i);
            i--;
        }
    }
    
    if( space_pressed and nanos-last_bullet_nano > 200000000){
        last_bullet_nano = nanos;
        Bullet bullet = Bullet();
        bullet.ref.x = me.ref.x;
        bullet.ref.y = me.ref.y;
        bullet.direction = me.direction;
        me.bullets.push_back(bullet);
    }
    
    if(pending_gamestate_change){
        pending_gamestate_change = false;
        std::swap(game_state,game_state_buffer);
    }
    last_frame_nano = nanos;
    glutSwapBuffers();
}

void read_map(){
    Point p;
    Rectangle rect;
    std::ifstream map_input("map.txt");
    map_input >> game_map->num_objects;
    for(int _i=0 ; _i<game_map->num_objects; ++_i){
        rect = Rectangle();
        map_input >> rect.red;
        map_input >> rect.green;
        map_input >> rect.blue;
        for( int j=0 ; j<4 ; j++ ){
            p = Point();
            map_input >> p.x >> p.y;
            rect.corners.push_back(p);
        }
        game_map->objects.push_back(rect);
    }
}

void reshape(int w, int h){
    WINDOW_WIDTH = w;
    WINDOW_HEIGHT = h;
    GAME_WINDOW_LEN = std::min(w,h);
    SHIFT_WIDTH=(w-GAME_WINDOW_LEN)/2;
    SHIFT_HEIGHT=(h-GAME_WINDOW_LEN)/2;
    glLineWidth(GAME_WINDOW_LEN/50);
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluOrtho2D (0, w, h, 0);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
}

void MouseMotion(int x,int y){
    double actual_x = SHIFT_WIDTH+GAME_WINDOW_LEN*((me.ref.x+1.0)/2.0);
    double actual_y = SHIFT_HEIGHT+GAME_WINDOW_LEN*((me.ref.y+1.0)/2.0);
    me.direction=atan2(y-actual_y,x-actual_x);
}

void keyboard(unsigned char key, int x, int y){
    switch (key) {
        case 'w':
            w_pressed = true;
            break;
        case 'a':
            a_pressed = true;
            break;
        case 's':
            s_pressed = true;
            break;
        case 'd':
            d_pressed = true;
            break;
        case ' ':
            space_pressed = true;
            break;
        case 27:
            std::exit(0);
            break;
        default:
            break;
    }
}

void keyboard_up(unsigned char key, int x, int y){
    switch (key) {
        case 'w':
            w_pressed = false;
            break;
        case 'a':
            a_pressed = false;
            break;
        case 's':
            s_pressed = false;
            break;
        case 'd':
            d_pressed = false;
            break;
        case ' ':
            space_pressed = false;
            break;
        case 27:
            std::exit(0);
            break;
        default:
            break;
    }
}

void special_key(int key, int x, int y){
    
}

void special_key_up(int key, int x, int y){
    
}

