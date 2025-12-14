#ifndef AUTH_H
#define AUTH_H
#include <stdbool.h>
char *check_login(const char *username, const char *password);
char *register_user(const char *username, const char *password);
#endif
