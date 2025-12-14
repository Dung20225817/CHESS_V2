#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 4096

// Định nghĩa kiểu hàm callback để Python truyền vào
// Python sẽ nhận dữ liệu dạng string (char*)
typedef void (*PythonCallback)(const char *data);

int sockfd = -1;
PythonCallback py_callback = NULL;
pthread_t recv_thread;
int is_running = 0;

// Hàm thread chạy ngầm để lắng nghe Server
void *receive_loop(void *arg) {
    char buf[BUF_SIZE];
    int n;
    while (is_running) {
        memset(buf, 0, BUF_SIZE);
        n = recv(sockfd, buf, BUF_SIZE - 1, 0);
        if (n <= 0) {
            if (py_callback) py_callback("{\"type\":\"DISCONNECT\"}");
            is_running = 0;
            break;
        }
        buf[n] = '\0';
        
        // Gọi hàm callback của Python để đẩy dữ liệu lên GUI
        if (py_callback) {
            py_callback(buf);
        }
    }
    return NULL;
}

// --- API CHO PYTHON GỌI ---

// 1. Hàm kết nối tới Server
int connect_server(const char *ip, int port, PythonCallback callback) {
    if (sockfd != -1) close(sockfd);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return 0; // Thất bại

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) return 0;

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return 0; // Kết nối lỗi
    }

    // Lưu callback và khởi động thread lắng nghe
    py_callback = callback;
    is_running = 1;
    if (pthread_create(&recv_thread, NULL, receive_loop, NULL) != 0) {
        return 0;
    }

    return 1; // Thành công
}

// 2. Hàm gửi dữ liệu (JSON string)
void send_msg(const char *msg) {
    if (sockfd != -1 && is_running) {
        send(sockfd, msg, strlen(msg), 0);
    }
}

// 3. Hàm đóng kết nối
void close_connection() {
    is_running = 0;
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}