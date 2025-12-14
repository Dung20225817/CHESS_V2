#ifndef UTILS_H
#define UTILS_H

#include "room.h"

// Đổi SOCKET -> int
void broadcast_room(Room *room, const char *msg);
void send_initial_state(int client, Room *room, char color);
char assign_player(Room *room, int client);

#endif