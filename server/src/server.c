#include "server.h"
#include "client_handler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int server_socket;
pthread_mutex_t cs;

// --- HÀM MỚI: BROADCAST UDP ---
void *broadcast_thread_func(void *arg) {
    int port = *(int*)arg;
    int sock;
    struct sockaddr_in broadcast_addr;
    char *msg = "CHESS_SERVER_HERE"; // Mật khẩu nhận diện
    int broadcast = 1;

    // 1. Tạo socket UDP
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Broadcast socket failed");
        return NULL;
    }

    // 2. Bật chế độ Broadcast
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("Broadcast setsockopt failed");
        close(sock);
        return NULL;
    }

    // 3. Cấu hình địa chỉ đích (255.255.255.255 là gửi cho tất cả mọi người)
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;
    broadcast_addr.sin_port = htons(6001); // Dùng cổng 6001 cho UDP (khác cổng 6000 của TCP)

    printf("[SERVER] Started broadcasting on port 6001...\n");

    // 4. Vòng lặp gửi tin
    while (1) {
        sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
        sleep(2); // Cứ 2 giây gửi 1 lần
    }

    close(sock);
    return NULL;
}

void init_server(int port) {
    // [MỚI] Bỏ qua lỗi SIGPIPE để server không bị crash khi client ngắt kết nối đột ngột
    signal(SIGPIPE, SIG_IGN);
    // 1. Khởi tạo Mutex (thay cho InitializeCriticalSection)
    if (pthread_mutex_init(&cs, NULL) != 0) {
        perror("Mutex init failed");
        exit(1);
    }

    // 2. Tạo socket (AF_INET = IPv4, SOCK_STREAM = TCP)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    // 3. Cho phép tái sử dụng cổng ngay lập tức (tránh lỗi "Address already in use")
    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
    }

    // 4. Bind socket
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // Lắng nghe mọi IP
    addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // 5. Listen
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    // THÊM ĐOẠN NÀY VÀO CUỐI HÀM init_server:
    pthread_t b_tid;
    int *udp_port = malloc(sizeof(int));
    *udp_port = port;
    pthread_create(&b_tid, NULL, broadcast_thread_func, udp_port);
    pthread_detach(b_tid);
    
    printf("[SERVER] Running on port %d (Linux)...\n", port);
}

void run_server() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        // Chấp nhận kết nối
        int client = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (client < 0) {
            perror("Accept failed");
            continue;
        }

        printf("[SERVER] New connection from %s\n", inet_ntoa(client_addr.sin_addr));

        // Cấp phát bộ nhớ cho client socket để truyền vào thread an toàn
        int *pclient = malloc(sizeof(int));
        *pclient = client;

        // Tạo thread mới (thay cho _beginthreadex)
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, (void *)pclient) != 0) {
            perror("Thread creation failed");
            close(client);
            free(pclient);
        } else {
            // Detach để hệ điều hành tự thu hồi tài nguyên khi thread kết thúc
            pthread_detach(tid);
        }
    }
}

void cleanup_server() {
    close(server_socket);
    pthread_mutex_destroy(&cs);
}