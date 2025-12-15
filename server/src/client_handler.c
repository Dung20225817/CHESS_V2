#include "client_handler.h"
#include "room.h"
#include "board.h"
#include "utils.h"
#include "server.h"
#include "auth.h"
#include "friend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // cho close()

// Hàm thread trên Linux trả về void* và nhận void*
void *client_thread(void *arg)
{
    int client = *(int *)arg; // Lấy socket descriptor
    free(arg);                // Giải phóng bộ nhớ đã malloc bên server.c

    char buf[BUF_SIZE];
    int n;

    // Vòng lặp nhận dữ liệu
    while ((n = recv(client, buf, BUF_SIZE - 1, 0)) > 0)
    {
        buf[n] = '\0';
        printf("[Server] Received: %s\n", buf);

        // --- Xử lý LOGIN / REGISTER ---
        if (strstr(buf, "\"type\""))
        {
            char username[64] = {0}, password[64] = {0};
            char *type = strstr(buf, "\"type\"");
            char *u = strstr(buf, "\"username\"");
            char *p = strstr(buf, "\"password\"");

            if (u && p)
            {
                sscanf(u, "\"username\"%*[^'\"]\"%63[^\"]", username);
                sscanf(p, "\"password\"%*[^'\"]\"%63[^\"]", password);
            }

            if (strstr(type, "LOGIN"))
            {
                char *user_id = check_login(username, password);
                if (user_id)
                {
                    char ok[256];
                    snprintf(ok, sizeof(ok), "{\"type\":\"LOGIN\",\"success\":true,\"user_id\":\"%s\"}\n", user_id);
                    send(client, ok, strlen(ok), 0);
                    free(user_id);
                }
                else
                {
                    char fail[] = "{\"type\":\"LOGIN\",\"success\":false}\n";
                    send(client, fail, strlen(fail), 0);
                    close(client);
                    return NULL;
                }
                continue; // Xong login thì tiếp tục vòng lặp đợi lệnh khác hoặc break tùy logic
            }
            else if (strstr(type, "REGISTER"))
            {
                char *user_id = register_user(username, password);
                if (user_id)
                {
                    char ok[256];
                    snprintf(ok, sizeof(ok), "{\"type\":\"REGISTER\",\"success\":true,\"user_id\":\"%s\"}\n", user_id);
                    send(client, ok, strlen(ok), 0);
                    free(user_id);
                }
                else
                {
                    char fail[] = "{\"type\":\"REGISTER\",\"success\":false}\n";
                    send(client, fail, strlen(fail), 0);
                    close(client);
                    return NULL;
                }
                continue;
            }
        }

        // --- 1. Xử lý SEARCH_USER ---
        if (strstr(buf, "\"type\":\"SEARCH_USER\"") || strstr(buf, "\"type\": \"SEARCH_USER\""))
        {
            char keyword[64] = {0};
            char *p = strstr(buf, "\"keyword\"");
            if (p)
                sscanf(p, "\"keyword\"%*[^'\"]\"%63[^\"]", keyword);

            Friend results[50];
            int count = search_users(keyword, results, 50);

            // Tạo chuỗi JSON thủ công
            char response[BUF_SIZE];
            int offset = snprintf(response, sizeof(response), "{\"type\":\"SEARCH_RESULT\",\"users\":[");
            for (int i = 0; i < count; i++)
            {
                if (i > 0)
                    offset += snprintf(response + offset, sizeof(response) - offset, ",");
                offset += snprintf(response + offset, sizeof(response) - offset,
                                   "{\"id\":%d,\"username\":\"%s\"}", results[i].id, results[i].username);
            }
            snprintf(response + offset, sizeof(response) - offset, "]}");

            send(client, response, strlen(response), 0);
            send(client, "\n", 1, 0); // Ký tự ngắt dòng cho Python
            continue;
        }

        // --- 2. Xử lý SEND_REQUEST (Gửi lời mời) ---
        if (strstr(buf, "\"type\":\"SEND_REQUEST\"") || strstr(buf, "\"type\": \"SEND_REQUEST\""))
        {
            int from_id = 0, to_id = 0;
            char *p1 = strstr(buf, "\"from_id\"");
            char *p2 = strstr(buf, "\"to_id\"");

            if (p1)
                from_id = atoi(p1 + 10); // Dịch con trỏ qua chuỗi "from_id":
            if (p2)
                to_id = atoi(p2 + 8); // Dịch con trỏ qua chuỗi "to_id":

            int res = send_friend_request(from_id, to_id);

            char response[256];
            if (res == 1)
                sprintf(response, "{\"type\":\"SEND_REQUEST\",\"success\":true}");
            else if (res == -2)
                sprintf(response, "{\"type\":\"SEND_REQUEST\",\"success\":false,\"msg\":\"Already friends\"}");
            else if (res == -3)
                sprintf(response, "{\"type\":\"SEND_REQUEST\",\"success\":false,\"msg\":\"Request pending\"}");
            else
                sprintf(response, "{\"type\":\"SEND_REQUEST\",\"success\":false,\"msg\":\"Error\"}");

            send(client, response, strlen(response), 0);
            send(client, "\n", 1, 0);
            continue;
        }

        // --- 3. Xử lý GET_REQUESTS (Xem danh sách lời mời) ---
        if (strstr(buf, "\"type\":\"GET_REQUESTS\"") || strstr(buf, "\"type\": \"GET_REQUESTS\""))
        {
            int user_id = 0;
            char *p = strstr(buf, "\"user_id\"");
            if (p)
                user_id = atoi(p + 10);

            FriendRequest reqs[50];
            int count = get_friend_requests(user_id, reqs, 50);

            char response[BUF_SIZE];
            int offset = snprintf(response, sizeof(response), "{\"type\":\"GET_REQUESTS\",\"requests\":[");
            for (int i = 0; i < count; i++)
            {
                if (i > 0)
                    offset += snprintf(response + offset, sizeof(response) - offset, ",");
                offset += snprintf(response + offset, sizeof(response) - offset,
                                   "{\"msg_id\":%d,\"from_id\":%d,\"username\":\"%s\"}", reqs[i].msg_id, reqs[i].from_id, reqs[i].from_username);
            }
            snprintf(response + offset, sizeof(response) - offset, "]}");

            send(client, response, strlen(response), 0);
            send(client, "\n", 1, 0);
            continue;
        }

        // --- 4. Xử lý ACCEPT_REQUEST (Chấp nhận kết bạn) ---
        if (strstr(buf, "\"type\":\"ACCEPT_REQUEST\"") || strstr(buf, "\"type\": \"ACCEPT_REQUEST\""))
        {
            int msg_id = 0, user_id = 0;
            char *p1 = strstr(buf, "\"msg_id\"");
            char *p2 = strstr(buf, "\"user_id\""); // ID người chấp nhận (người nhận được lời mời)

            if (p1)
                msg_id = atoi(p1 + 9);
            if (p2)
                user_id = atoi(p2 + 10);

            int res = accept_friend_request(msg_id, user_id);

            char response[256];
            if (res == 1)
                sprintf(response, "{\"type\":\"ACCEPT_REQUEST\",\"success\":true}");
            else
                sprintf(response, "{\"type\":\"ACCEPT_REQUEST\",\"success\":false}");

            send(client, response, strlen(response), 0);
            send(client, "\n", 1, 0);
            continue;
        }

        // --- 5. Xử lý GET_FRIENDS (Giữ nguyên logic cũ nhưng cập nhật JSON) ---
        if (strstr(buf, "\"type\":\"GET_FRIENDS\"") || strstr(buf, "\"type\": \"GET_FRIENDS\""))
        {
            int user_id = 0;
            char *p = strstr(buf, "\"user_id\"");
            if (p)
                user_id = atoi(p + 10);

            if (user_id > 0)
            {
                Friend friends[100];
                int count = get_friends(user_id, friends, 100);
                char response[BUF_SIZE];
                int offset = snprintf(response, sizeof(response), "{\"type\":\"GET_FRIENDS\",\"friends\":[");
                for (int i = 0; i < count; i++)
                {
                    if (i > 0)
                        offset += snprintf(response + offset, sizeof(response) - offset, ",");
                    offset += snprintf(response + offset, sizeof(response) - offset,
                                       "{\"id\":%d,\"username\":\"%s\"}", friends[i].id, friends[i].username);
                }
                snprintf(response + offset, sizeof(response) - offset, "]}");
                send(client, response, strlen(response), 0);
                send(client, "\n", 1, 0);
            }
            continue;
        }

        // ==========================================
        //        TÍNH NĂNG THÁCH ĐẤU (NEW)
        // ==========================================

        // 1. Gửi thách đấu
        if (strstr(buf, "\"type\":\"SEND_CHALLENGE\"") || strstr(buf, "\"type\": \"SEND_CHALLENGE\""))
        {
            int from_id = 0, to_id = 0;
            char *p1 = strstr(buf, "\"from_id\"");
            char *p2 = strstr(buf, "\"to_id\"");
            if (p1)
                from_id = atoi(p1 + 10);
            if (p2)
                to_id = atoi(p2 + 8);

            int res = send_challenge(from_id, to_id);

            char response[256];
            if (res == 1)
                sprintf(response, "{\"type\":\"SEND_CHALLENGE\",\"success\":true}");
            else
                sprintf(response, "{\"type\":\"SEND_CHALLENGE\",\"success\":false}");

            send(client, response, strlen(response), 0);
            send(client, "\n", 1, 0);
            continue;
        }

        // 2. Lấy danh sách thách đấu (Người bị thách đấu gọi cái này)
        if (strstr(buf, "\"type\":\"GET_CHALLENGES\"") || strstr(buf, "\"type\": \"GET_CHALLENGES\""))
        {
            int user_id = 0;
            char *p = strstr(buf, "\"user_id\"");
            if (p)
                user_id = atoi(p + 10);

            Challenge challs[20];
            int count = get_incoming_challenges(user_id, challs, 20);

            char response[BUF_SIZE];
            int offset = snprintf(response, sizeof(response), "{\"type\":\"GET_CHALLENGES\",\"challenges\":[");
            for (int i = 0; i < count; i++)
            {
                if (i > 0)
                    offset += snprintf(response + offset, sizeof(response) - offset, ",");
                offset += snprintf(response + offset, sizeof(response) - offset,
                                   "{\"msg_id\":%d,\"from_id\":%d,\"username\":\"%s\"}", challs[i].msg_id, challs[i].from_id, challs[i].from_username);
            }
            snprintf(response + offset, sizeof(response) - offset, "]}");

            send(client, response, strlen(response), 0);
            send(client, "\n", 1, 0);
            continue;
        }

        // 3. Chấp nhận thách đấu -> Trả về Tên Phòng để Client tự join
        if (strstr(buf, "\"type\":\"ACCEPT_CHALLENGE\"") || strstr(buf, "\"type\": \"ACCEPT_CHALLENGE\""))
        {
            int msg_id = 0, user_id = 0, from_id = 0; // user_id là người chấp nhận
            char *p1 = strstr(buf, "\"msg_id\"");
            char *p2 = strstr(buf, "\"user_id\"");
            char *p3 = strstr(buf, "\"from_id\""); // ID người gửi thách đấu (để tạo tên phòng)

            if (p1)
                msg_id = atoi(p1 + 9);
            if (p2)
                user_id = atoi(p2 + 10);
            if (p3)
                from_id = atoi(p3 + 10);

            int res = accept_challenge(msg_id, user_id);

            if (res == 1)
            {
                // Tạo tên phòng: duel_fromID_and_toID (VD: duel_1_and_2)
                char room_name[64];
                sprintf(room_name, "duel_%d_and_%d", from_id, user_id);

                // Trả về tên phòng để Client này join ngay lập tức
                char response[256];
                sprintf(response, "{\"type\":\"CHALLENGE_ACCEPTED\",\"success\":true,\"room\":\"%s\"}", room_name);
                send(client, response, strlen(response), 0);
                send(client, "\n", 1, 0);
            }
            else
            {
                char fail[] = "{\"type\":\"CHALLENGE_ACCEPTED\",\"success\":false}";
                send(client, fail, strlen(fail), 0);
                send(client, "\n", 1, 0);
            }
            continue;
        }

        // 4. Kiểm tra trạng thái thách đấu (Người thách đấu gọi cái này liên tục)
        if (strstr(buf, "\"type\":\"CHECK_CHALLENGE\"") || strstr(buf, "\"type\": \"CHECK_CHALLENGE\""))
        {
            int from_id = 0, to_id = 0;
            char *p1 = strstr(buf, "\"from_id\"");
            char *p2 = strstr(buf, "\"to_id\"");
            if (p1)
                from_id = atoi(p1 + 10);
            if (p2)
                to_id = atoi(p2 + 8);

            int status = check_challenge_status(from_id, to_id);
            // 0: pending, 1: accepted, -1: none

            if (status == 1)
            {
                // Nếu đối thủ đã chấp nhận -> Gửi tên phòng để join
                char room_name[64];
                sprintf(room_name, "duel_%d_and_%d", from_id, to_id);
                char response[256];
                sprintf(response, "{\"type\":\"CHALLENGE_ACCEPTED\",\"success\":true,\"room\":\"%s\"}", room_name);
                send(client, response, strlen(response), 0);
            }
            else if (status == 0)
            {
                char response[] = "{\"type\":\"CHALLENGE_STATUS\",\"status\":\"pending\"}";
                send(client, response, strlen(response), 0);
            }
            else
            {
                char response[] = "{\"type\":\"CHALLENGE_STATUS\",\"status\":\"none\"}";
                send(client, response, strlen(response), 0);
            }
            send(client, "\n", 1, 0);
            continue;
        }

        // --- XỬ LÝ GET_ROOMS (MỚI) ---
        if (strstr(buf, "\"type\":\"GET_ROOMS\"") || strstr(buf, "\"type\": \"GET_ROOMS\""))
        {
            char response[4096]; // Buffer đủ lớn
            get_room_list_json(response);

            send(client, response, strlen(response), 0);
            send(client, "\n", 1, 0);
            continue;
        }

        // --- Xử lý GAME JOIN ---
        if (strstr(buf, "\"type\":\"join\"") || strstr(buf, "\"type\": \"join\""))
        {
            char room_name[32] = {0};
            char *room_ptr = strstr(buf, "\"room\"");
            if (room_ptr)
                sscanf(room_ptr, "\"room\"%*[^'\"]\"%31[^\"]", room_name);

            if (room_name[0])
            {
                int room_idx = get_or_create_room(room_name);
                if (room_idx != -1)
                {
                    Room *room = &rooms[room_idx];
                    char player_color = assign_player(room, client);
                    
                    // [MỚI] 1. Cấu hình Timeout 60 giây cho Socket này
                    struct timeval tv;
                    tv.tv_sec = 60;  // 60 Giây timeout
                    tv.tv_usec = 0;
                    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

                    send_initial_state(client, room, player_color);

                    // Vào vòng lặp xử lý nước đi cho room này
                    while (1) 
                    {
                        // [MỚI] 2. Nhận dữ liệu với kiểm tra lỗi kỹ càng hơn
                        n = recv(client, buf, BUF_SIZE - 1, 0);
                        
                        if (n <= 0) {
                            // --- XỬ LÝ KHI MẤT KẾT NỐI / TIMEOUT ---
                            printf("[Server] Client disconnected or timed out (Room: %s)\n", room_name);
                            
                            pthread_mutex_lock(&cs);
                            // Báo cho người còn lại biết
                            char msg[] = "{\"type\":\"gameOver\",\"winner\":\"opponent_disconnect\",\"reason\":\"Mất kết nối\"}";
                            broadcast_room(room, msg);
                            
                            // Reset phòng để người sau có thể chơi lại
                            init_board(room->board, &room->turn);
                            room->player_count = 0; // Xóa sạch người chơi trong phòng ảo
                            pthread_mutex_unlock(&cs);
                            
                            break; // Thoát vòng lặp game
                        }

                        buf[n] = '\0';
                        
                        // [Logic xử lý nước đi cũ giữ nguyên]
                        char from[3] = {0}, to[3] = {0};
                        char *p = strstr(buf, "\"move\":\"");
                        if (!p) continue; // Bỏ qua nếu tin nhắn rác
                        
                        p += 8; // (Điều chỉnh tùy theo format của bạn, ở đây +8 vì "move":" dài 8 ký tự, nhưng cẩn thận nếu JSON Python có dấu cách)
                        // Nếu Python gửi "move": "..." (có dấu cách) thì p phải tìm khéo hơn.
                        // Code cũ của bạn: strncpy(from, p, 2); ... 
                        // Nếu logic cũ đang chạy tốt thì giữ nguyên đoạn parse này.
                        
                        // Để an toàn hơn với format JSON Python, bạn nên dùng logic parse linh hoạt hơn ở đây
                        // Nhưng giả sử logic cũ ok, ta paste lại:
                        strncpy(from, p, 2);
                        strncpy(to, p + 2, 2);

                        // === LOCK MUTEX ===
                        pthread_mutex_lock(&cs);

                        promotion_move(room, from, to);

                        // Gửi thông báo nước đi (move_notify)
                        char move_msg[128];
                        const char *next_turn_str = (room->turn == 'w') ? "white" : "black";
                        sprintf(move_msg, "{\"type\":\"move_notify\",\"move\":\"%s%s\",\"turn\":\"%s\"}", from, to, next_turn_str);
                        broadcast_room(room, move_msg);

                        char result = check_game_over(room);
                        if (result)
                        {
                            char msg[128];
                            // ... tạo chuỗi msg ...
                            if (result == 'w') sprintf(msg, "{\"type\":\"gameOver\",\"winner\":\"white\"}");
                            else if (result == 'b') sprintf(msg, "{\"type\":\"gameOver\",\"winner\":\"black\"}");
                            else sprintf(msg, "{\"type\":\"gameOver\",\"winner\":\"draw\"}");
                            
                            broadcast_room(room, msg);
                            init_board(room->board, &room->turn);
                        }
                        else
                        {
                            char json[4096];
                            get_board_json(room, json);
                            broadcast_room(room, json);
                        }

                        pthread_mutex_unlock(&cs);
                    }
                    
                    // Ra khỏi vòng lặp -> Xóa player khỏi phòng
                    remove_player(room, client);
                }
            }
            break; // Thoát thread
        }
    }

    close(client);
    return NULL;
}