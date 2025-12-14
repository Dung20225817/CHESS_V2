#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h> // cần cho send()
#include "board.h"

char assign_player(Room *room, int client) { // SOCKET -> int
    char color = (room->player_count == 0) ? 'w' : 'b';
    room->players[room->player_count].sock = client;
    room->players[room->player_count].color = color;
    room->player_count++;
    return color;
}

void send_initial_state(int client, Room *room, char color) { // SOCKET -> int
    char msg[256];
    sprintf(msg, "{\"type\":\"assignColor\",\"color\":\"%s\"}", (color == 'w') ? "white" : "black");
    send(client, msg, (int)strlen(msg), 0);
    send(client, "\n", 1, 0);

    char json[4096];
    get_board_json(room, json);
    send(client, json, (int)strlen(json), 0);
    send(client, "\n", 1, 0);
}

// Bổ sung hàm broadcast (vì nó được khai báo trong utils.h/room.h nhưng chưa thấy implement ở file server cũ)
void broadcast_room(Room *room, const char *msg) {
    for (int i = 0; i < room->player_count; i++) {
        send(room->players[i].sock, msg, strlen(msg), 0);
        send(room->players[i].sock, "\n", 1, 0);
    }
}