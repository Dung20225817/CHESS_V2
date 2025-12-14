#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>     // Cho close()
#include <pthread.h>    // Cho mutex và thread
#include <stdio.h>
#include <stdlib.h>

// Định nghĩa INVALID_SOCKET cho Linux để tương thích logic cũ
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

extern int server_socket;      // Đổi SOCKET -> int
extern pthread_mutex_t cs;     // Đổi CRITICAL_SECTION -> pthread_mutex_t

void init_server(int port);
void run_server();
void cleanup_server();
void *broadcast_thread_func(void *arg);


#endif