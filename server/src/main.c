#include "server.h"

int main() {
    init_server(6000);
    run_server();
    cleanup_server();
    return 0;
}
