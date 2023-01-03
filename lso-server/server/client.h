#ifndef __H_CLIENT__
#define __H_CLIENT__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <mysql/mysql.h>

/* User structure */
typedef struct {
    char username[50];
    char password[50];
    char accessibility[20];
    char flag[50];
} User;

/* Client structure */
typedef struct {
    struct sockaddr_in address;
    int sockfd;
    int uid;
    User user;
} client_t;

void *handle_client(void *arg);
void queue_add(client_t *client);
void queue_remove(int uid);
void send_message(char *s, int uid);

#endif
