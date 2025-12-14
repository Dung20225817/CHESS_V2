#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <pthread.h>

#define BUF_SIZE 4096

// Thay đổi kiểu trả về và tham số cho đúng chuẩn pthread
void *client_thread(void *arg);

#endif