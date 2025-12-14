#include "board.h"
#include <string.h>
#include <stdio.h>

void init_board(char board[8][8], char *turn) {
    const char *init[8] = {
        "rnbqkbnr","pppppppp","........","........",
        "........","........","PPPPPPPP","RNBQKBNR"};
    for (int i=0;i<8;i++)
        for (int j=0;j<8;j++)
            board[i][j]=init[i][j];
    *turn='w';
}

void promotion_move(Room *room, char *from, char *to) {
    int fr = 8 - (from[1] - '0');
    int fc = from[0] - 'a';
    int tr = 8 - (to[1] - '0');
    int tc = to[0] - 'a';
    char piece = room->board[fr][fc];
    room->board[fr][fc] = '.';
    room->board[tr][tc] = piece;

    if (piece == 'P' && tr == 0) room->board[tr][tc] = 'Q';
    if (piece == 'p' && tr == 7) room->board[tr][tc] = 'q';
    room->turn = (room->turn == 'w') ? 'b' : 'w';
}

char check_game_over(Room *room) {
    int wk=0,bk=0;
    for (int i=0;i<8;i++)
        for (int j=0;j<8;j++) {
            if (room->board[i][j]=='K') wk=1;
            if (room->board[i][j]=='k') bk=1;
        }
    if (!wk && bk) return 'b';
    if (wk && !bk) return 'w';
    if (!wk && !bk) return 'd';
    return 0;
}

void get_board_json(Room *room, char *json) {
    strcpy(json,"{\"type\":\"state\",\"fen\":\"");
    for (int i=0;i<8;i++){
        for (int j=0;j<8;j++)
            strncat(json,&room->board[i][j],1);
        if(i<7) strcat(json,"/");
    }
    strcat(json,"\",\"turn\":\"");
    strcat(json,(room->turn=='w')?"white":"black");
    strcat(json,"\"}");
}
