// ...existing code...
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

char *check_login(const char *username, const char *password) {
    FILE *f = fopen("accounts.txt", "r");
    if (!f) {
        perror("Cannot open accounts.txt");
        return NULL;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        // loại bỏ ký tự xuống dòng nếu có
        line[strcspn(line, "\r\n")] = 0;

        // bỏ qua dòng trống
        if (strlen(line) == 0) continue;

        // bỏ ký tự BOM nếu có (Notepad UTF-8)
        if (strlen(line) >= 3 &&
            (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF) {
            memmove(line, line + 3, strlen(line + 3) + 1);
        }

        char user_id[64], user[64], pass[64];
        if (sscanf(line, "%63s %63s %63s", user_id, user, pass) == 3) {
            if (strcmp(user, username) == 0 && strcmp(pass, password) == 0) {
                // trả về user_id (cần free() ở caller)
                char *ret = malloc(strlen(user_id) + 1);
                if (ret) strcpy(ret, user_id);
                fclose(f);
                return ret;
            }
        }
    }

    fclose(f);
    return NULL;
}

char *register_user(const char *username, const char *password) {
    // Kiểm tra username đã tồn tại và tìm user_id lớn nhất
    FILE *f = fopen("accounts.txt", "r");
    int max_id = 0;
    
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char user_id[64], user[64], pass[64];
            if (sscanf(line, "%63s %63s %63s", user_id, user, pass) == 3) {
                // Kiểm tra trùng username
                if (strcmp(user, username) == 0) {
                    fclose(f);
                    return NULL;
                }
                // Tìm id lớn nhất
                int current_id;
                if (sscanf(user_id, "%d", &current_id) == 1) {
                    if (current_id > max_id) {
                        max_id = current_id;
                    }
                }
            }
        }
        fclose(f);
    }

    // Tạo user_id mới = max_id + 1
    char user_id[64];
    snprintf(user_id, sizeof(user_id), "%d", max_id + 1);

    // Thêm tài khoản mới vào file
    f = fopen("accounts.txt", "a");
    if (!f) {
        perror("Cannot open accounts.txt");
        return NULL;
    }

    fprintf(f, "%s %s %s\n", user_id, username, password);
    fclose(f);

    // Trả về user_id (caller phải free)
    char *ret = malloc(strlen(user_id) + 1);
    if (ret) strcpy(ret, user_id);
    return ret;
}
// ...existing code...