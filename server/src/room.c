#include "room.h"
#include <stdio.h>
#include "board.h" // Để dùng init_board

Room rooms[MAX_ROOMS];
int room_count = 0;

int get_or_create_room(const char *room_name) {
    for (int i = 0; i < room_count; i++) {
        if (strcmp(rooms[i].name, room_name) == 0) {
            if (rooms[i].player_count < MAX_CLIENTS) {
                return i;
            } else {
                return -1; // Phòng đầy
            }
        }
    }
    
    if (room_count < MAX_ROOMS) {
        strcpy(rooms[room_count].name, room_name);
        rooms[room_count].player_count = 0;
        init_board(rooms[room_count].board, &rooms[room_count].turn);
        return room_count++;
    }
    return -1; // Server đầy
}

void remove_player(Room *room, int client) { // SOCKET -> int
    for (int i = 0; i < room->player_count; i++) {
        if (room->players[i].sock == client) {
            // Dịch chuyển mảng nếu cần (ở đây max 2 người nên đơn giản)
            if (i == 0 && room->player_count > 1) {
                room->players[0] = room->players[1];
            }
            room->player_count--;
            break;
        }
    }
}

void get_room_list_json(char *json) {
    // Khởi tạo JSON
    strcpy(json, "{\"type\":\"ROOM_LIST\",\"rooms\":[");
    
    int offset = strlen(json);
    int has_room = 0;

    for (int i = 0; i < room_count; i++) {
        // Chỉ hiện phòng có người nhưng chưa đầy (hoặc hiện tất cả tùy bạn)
        // Ở đây ta hiện tất cả phòng đang tồn tại
        if (i > 0) {
            offset += sprintf(json + offset, ",");
        }
        
        // Format: {"name": "TênPhòng", "count": SốNgười}
        offset += sprintf(json + offset, "{\"name\":\"%s\",\"count\":%d}", 
                          rooms[i].name, rooms[i].player_count);
        has_room = 1;
    }

    strcat(json, "]}");
}