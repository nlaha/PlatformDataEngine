#pragma once

#include <winsock2.h>
#include <WS2tcpip.h>

// this can be whatever we want as long as client and server are using the same port
#define PORT "8888"
#define BUFFER_LENGTH 512


/*
Code for client socket - this should eventually allow a player to "join" a game
*/

int basic_connection(char* ip_address);