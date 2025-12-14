#ifndef BOARD_H
#define BOARD_H

#include "room.h"

void init_board(char board[8][8], char *turn);
void promotion_move(Room *room, char *from, char *to);
char check_game_over(Room *room);
void get_board_json(Room *room, char *json);

#endif
