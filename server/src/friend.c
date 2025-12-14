#include "friend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FRIEND_FILE "friend.txt"
#define ACCOUNT_FILE "accounts.txt" // Đã sửa cho khớp với auth.c
#define MESSAGE_FILE "messages.txt"

// --- Helper Functions ---

int get_username_from_id(int user_id, char *username) {
    FILE *fp = fopen(ACCOUNT_FILE, "r");
    if (!fp) return 0;
    
    int id;
    char name[32], pass[32];
    // Format: ID Username Password
    while (fscanf(fp, "%d %31s %31s", &id, name, pass) == 3) {
        if (id == user_id) {
            strcpy(username, name);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int get_next_message_id() {
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (!fp) return 1;
    int max_id = 0, id;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d", &id) == 1) {
            if (id > max_id) max_id = id;
        }
    }
    fclose(fp);
    return max_id + 1;
}

int get_next_friend_stt() {
    FILE *fp = fopen(FRIEND_FILE, "r");
    if (!fp) return 1;
    int max_stt = 0, stt;
    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "%d", &stt) == 1) {
            if (stt > max_stt) max_stt = stt;
        }
    }
    fclose(fp);
    return max_stt + 1;
}

// Kiểm tra đã là bạn chưa
int are_friends(int user_id1, int user_id2) {
    FILE *fp = fopen(FRIEND_FILE, "r");
    if (!fp) return 0;
    int stt, a, b;
    while (fscanf(fp, "%d %d %d", &stt, &a, &b) == 3) {
        if ((a == user_id1 && b == user_id2) || (a == user_id2 && b == user_id1)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// --- Main Features ---

// 1. Tìm kiếm người dùng
int search_users(const char *keyword, Friend *results, int max_size) {
    FILE *fp = fopen(ACCOUNT_FILE, "r");
    if (!fp) return 0;

    int count = 0;
    int id;
    char name[32], pass[32];

    while (fscanf(fp, "%d %31s %31s", &id, name, pass) == 3) {
        // Tìm chuỗi con (keyword trong name)
        if (strstr(name, keyword) != NULL) {
            if (count < max_size) {
                results[count].id = id;
                strcpy(results[count].username, name);
                count++;
            }
        }
    }
    fclose(fp);
    return count;
}

// 2. Lấy danh sách bạn bè
int get_friends(int user_id, Friend *friends, int max_size) {
    FILE *fp = fopen(FRIEND_FILE, "r");
    if (!fp) return 0;

    int count = 0;
    int stt, a, b;
    while (fscanf(fp, "%d %d %d", &stt, &a, &b) == 3) {
        if (a == user_id || b == user_id) {
            int friend_id = (a == user_id) ? b : a;
            char name[32];
            if (get_username_from_id(friend_id, name)) {
                if (count < max_size) {
                    friends[count].id = friend_id;
                    strcpy(friends[count].username, name);
                    count++;
                }
            }
        }
    }
    fclose(fp);
    return count;
}

// 3. Gửi lời mời kết bạn
int send_friend_request(int from_id, int to_id) {
    if (from_id == to_id) return -1; // Không tự kết bạn
    if (are_friends(from_id, to_id)) return -2; // Đã là bạn

    // Kiểm tra xem đã gửi request chưa (tránh spam)
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (fp) {
        int mid, f, t;
        char type[32], status[32], content[256];
        long time;
        while (fscanf(fp, "%d %d %d %s %s %ld %[^\n]", &mid, &f, &t, type, status, &time, content) == 7) {
            if (f == from_id && t == to_id && strcmp(type, "friend_request") == 0 && strcmp(status, "pending") == 0) {
                fclose(fp);
                return -3; // Đã gửi rồi, đang chờ
            }
        }
        fclose(fp);
    }

    // Ghi request mới vào messages.txt
    fp = fopen(MESSAGE_FILE, "a");
    if (!fp) return 0;

    int msg_id = get_next_message_id();
    long now = time(NULL);
    // Format: ID From To Type Status Time Content
    fprintf(fp, "%d %d %d friend_request pending %ld Request\n", msg_id, from_id, to_id, now);
    fclose(fp);
    return 1;
}

// 4. Lấy danh sách lời mời (Pending) cho user_id
int get_friend_requests(int user_id, FriendRequest *requests, int max_size) {
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (!fp) return 0;

    int count = 0;
    int mid, f, t;
    char type[32], status[32], content[256];
    long time;

    while (fscanf(fp, "%d %d %d %s %s %ld %[^\n]", &mid, &f, &t, type, status, &time, content) == 7) {
        // Nếu người nhận là user_id VÀ là request pending
        if (t == user_id && strcmp(type, "friend_request") == 0 && strcmp(status, "pending") == 0) {
            char sender_name[32];
            if (get_username_from_id(f, sender_name)) {
                if (count < max_size) {
                    requests[count].msg_id = mid;
                    requests[count].from_id = f;
                    strcpy(requests[count].from_username, sender_name);
                    count++;
                }
            }
        }
    }
    fclose(fp);
    return count;
}

// 5. Chấp nhận lời mời
int accept_friend_request(int msg_id, int user_id) {
    // Đọc toàn bộ file vào RAM để sửa dòng tương ứng
    // Lưu ý: Cách này đơn giản cho bài tập, với hệ thống lớn cần database
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (!fp) return 0;

    // Bộ nhớ tạm để lưu file
    char buffer[1024][256]; 
    int line_count = 0;
    int from_id = -1, to_id = -1;
    int found = 0;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        int mid, f, t;
        char type[32], status[32];
        // Parse sơ bộ để check ID
        if (sscanf(line, "%d %d %d %s %s", &mid, &f, &t, type, status) == 5) {
            if (mid == msg_id && t == user_id && strcmp(type, "friend_request") == 0 && strcmp(status, "pending") == 0) {
                // Tìm thấy request -> Sửa status thành accepted
                // Giữ nguyên timestamp và content
                char content[256];
                long time_val;
                sscanf(line, "%*d %*d %*d %*s %*s %ld %[^\n]", &time_val, content);
                
                sprintf(buffer[line_count], "%d %d %d friend_request accepted %ld %s\n", mid, f, t, time_val, content);
                from_id = f;
                to_id = t;
                found = 1;
            } else {
                strcpy(buffer[line_count], line);
            }
        } else {
             strcpy(buffer[line_count], line);
        }
        line_count++;
    }
    fclose(fp);

    if (!found) return 0;

    // Ghi lại file messages.txt
    fp = fopen(MESSAGE_FILE, "w");
    for (int i = 0; i < line_count; i++) {
        fputs(buffer[i], fp);
    }
    fclose(fp);

    // Thêm vào friend.txt
    fp = fopen(FRIEND_FILE, "a");
    int stt = get_next_friend_stt();
    fprintf(fp, "\n%d %d %d", stt, from_id, to_id);
    fclose(fp);

    return 1;
}

// ==========================================================
// --- PHẦN THÊM MỚI: TÍNH NĂNG THÁCH ĐẤU ---
// ==========================================================

int send_challenge(int from_id, int to_id) {
    if (from_id == to_id) return -1;
    
    // Kiểm tra xem có đang pending challenge nào không
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (fp) {
        int mid, f, t; char type[32], status[32], content[256]; long time;
        while (fscanf(fp, "%d %d %d %s %s %ld %[^\n]", &mid, &f, &t, type, status, &time, content) == 7) {
            if (f == from_id && t == to_id && strcmp(type, "challenge") == 0 && strcmp(status, "pending") == 0) {
                fclose(fp);
                return -3; // Đang chờ đối thủ trả lời, không gửi thêm
            }
        }
        fclose(fp);
    }

    fp = fopen(MESSAGE_FILE, "a");
    if (!fp) return 0;
    int msg_id = get_next_message_id();
    long now = time(NULL);
    // Ghi tin nhắn loại "challenge"
    fprintf(fp, "%d %d %d challenge pending %ld Duel?\n", msg_id, from_id, to_id, now);
    fclose(fp);
    return 1;
}

int get_incoming_challenges(int user_id, Challenge *challenges, int max_size) {
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (!fp) return 0;

    int count = 0;
    int mid, f, t; char type[32], status[32], content[256]; long time;

    while (fscanf(fp, "%d %d %d %s %s %ld %[^\n]", &mid, &f, &t, type, status, &time, content) == 7) {
        // Lọc tin nhắn: người nhận là user_id, loại challenge, trạng thái pending
        if (t == user_id && strcmp(type, "challenge") == 0 && strcmp(status, "pending") == 0) {
            char name[32];
            if (get_username_from_id(f, name)) {
                if (count < max_size) {
                    challenges[count].msg_id = mid;
                    challenges[count].from_id = f;
                    strcpy(challenges[count].from_username, name);
                    count++;
                }
            }
        }
    }
    fclose(fp);
    return count;
}

int accept_challenge(int msg_id, int user_id) {
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (!fp) return 0;

    char buffer[1024][256];
    int line_count = 0;
    int found = 0;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        int mid, f, t; char type[32], status[32];
        if (sscanf(line, "%d %d %d %s %s", &mid, &f, &t, type, status) == 5) {
            // Xác nhận đúng ID tin nhắn và đúng người nhận
            if (mid == msg_id && t == user_id && strcmp(type, "challenge") == 0 && strcmp(status, "pending") == 0) {
                char content[256]; long time_val;
                sscanf(line, "%*d %*d %*d %*s %*s %ld %[^\n]", &time_val, content);
                
                // Đổi trạng thái sang ACCEPTED
                sprintf(buffer[line_count], "%d %d %d challenge accepted %ld %s\n", mid, f, t, time_val, content);
                found = 1;
            } else {
                strcpy(buffer[line_count], line);
            }
        } else {
             strcpy(buffer[line_count], line);
        }
        line_count++;
    }
    fclose(fp);

    if (!found) return 0;

    // Ghi đè lại file
    fp = fopen(MESSAGE_FILE, "w");
    for (int i = 0; i < line_count; i++) {
        fputs(buffer[i], fp);
    }
    fclose(fp);
    return 1;
}

// Hàm này dành cho người thách đấu (Sender) kiểm tra xem đối thủ đã chấp nhận chưa
int check_challenge_status(int from_id, int to_id) {
    FILE *fp = fopen(MESSAGE_FILE, "r");
    if (!fp) return -1;

    int mid, f, t; char type[32], status[32], content[256]; long time;
    int result = 0; // 0: pending, 1: accepted, -1: not found

    // Tìm tin nhắn challenge mới nhất giữa 2 người
    while (fscanf(fp, "%d %d %d %s %s %ld %[^\n]", &mid, &f, &t, type, status, &time, content) == 7) {
        if (f == from_id && t == to_id && strcmp(type, "challenge") == 0) {
            if (strcmp(status, "accepted") == 0) {
                result = 1;
            } else if (strcmp(status, "pending") == 0) {
                result = 0;
            }
        }
    }
    fclose(fp);
    return result;
}