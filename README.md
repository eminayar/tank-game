# tank-game
LAN multiplayer tank game

```
g++ server.cpp gameState.h -std=c++14 -o server

./server <server_ip_address>


compile everything except server.cpp linking glut and opengl

./tankgame <client_ip_address>
```

alternatively open as an xcode project and add buildtime linking with glut and opengl, also make xcode to move map.txt to target location and add your ip address runtime arguments.

