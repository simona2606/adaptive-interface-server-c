#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CLIENTS 6
#define BUFFER_SZ 2048

static int uid = 10;

typedef struct {
  char username[50];
  char password[50];
  char accessibility[20];
} User;


typedef struct {
    struct sockaddr_in address;
    int sockfd;
    int uid;
    User user;
} client_t;


client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

MYSQL *start_connection(MYSQL *conn);
MYSQL_RES *send_query_insert(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1);
MYSQL_RES *send_query_select(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1);
void close_connection(MYSQL *conn, MYSQL_RES *res);

void str_trim_lf (char* arr, int length);

void *handle_client(void *arg);
void send_message(char *s, char* username);
void queue_add(client_t *client);

void start_socket();
bool registration(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row);
bool login(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row);


int main() {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = start_connection(conn);
    start_socket();
  /*
    if(registration(&this_u1, conn, res, row)){
    send_message();
    flag_login = login(&this_u1, conn, res, row);
      printf("\nFlag: %d\n", flag_login);
  }
  else{
    perror("ERROR: Registration Failed!!!");
  }
  */

    //close_connection(conn, res);
}

void start_socket() {
    system("clear");
    
    pthread_t tid;
    struct sockaddr_in client_addr;
    
        int serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if(serverfd == -1) {
            perror("Socket failed\n");
            exit(0);
        }

        struct sockaddr_in server, client;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(5008);
        server.sin_family = AF_INET;

        int b = bind(serverfd, (struct sockaddr *) &server, sizeof(server));

        if(b == -1) {
            perror("Bind failure\n");
            exit(0);
        }

        int i = listen(serverfd, 5);

        if(i == 0) {
            printf("i = %d\n", i);
            printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
        }

        printf("prima size\n");
        socklen_t size = sizeof(server);
        printf("dopo size\n");
        while (1) {
        	printf("Nel while\n");
            	int clientfd = accept(serverfd, (struct sockaddr *) &client, &size);

            	if(clientfd == -1) {
                    perror("Accept failure\n");
                    exit(0);
            	}
            
            	printf("CLIENT create QUEUE\n");

            	client_t* client = (client_t *)malloc(sizeof(client_t));
            	client->address = client_addr;
            	client->sockfd = clientfd;
            
            	printf("CLIEN QUEUE\n");
            	queue_add(client);
            	printf("CLIEN QUEUE DOPOOO\n");
            
            	pthread_create(&tid, NULL, &handle_client, (void*)client);
        }
}


void *handle_client(void *arg) {
        char buff_out[BUFFER_SZ];
        User* user = (User *) malloc(sizeof(User));
        
        int leave_flag = 0;
        bool flag_login = false;
        
        MYSQL *conn;
    	MYSQL_RES *res;
    	MYSQL_ROW row;

        //cli_count++;
        printf("creiamo la struct\n");
        client_t *client = (client_t *)arg;
        //sendNumberClient(cli_count);

        // Name
        printf("Prima if client\n");
        if(recv(client->sockfd, user, 32, 0) <= 0) {
            printf("Error.\n");
            leave_flag = 1;
        } else {
            //strcpy(client->user->username, user->name);
            
            
            printf("copia client\n");
            client->user = *user;
            
            printf("client copiato\n");
            
            //sprintf(buff_out, "%s has joined\n", client->name);
            //printf("%s", buff_out);
            //send_message(buff_out, client->uid);
        }

        bzero(buff_out, BUFFER_SZ);

        while(1) {
            if (leave_flag) {
                    break;
            }

            int receive = recv(client->sockfd, buff_out, BUFFER_SZ, 0);
            printf("dopo receive = %d\n", receive);
            if (receive > 0) {
            printf("Maronn\n");
                    if(strlen(buff_out) > 0) {
                        if(strlen(client->user.username) == 0) {
                            strcpy(client->user.username, buff_out);
                        } else if(strlen(client->user.password) == 0) {
                            strcpy(client->user.password, buff_out);
                        } else if(strlen(client->user.accessibility) == 0) {
                            strcpy(client->user.accessibility, buff_out);
                        }
                    
                        User user = client->user;
                        printf("Client user bho\n");
                        //send_message(buff_out, cli->uid);
                        printf("Ok\n");
                        if(strlen(user.username) == 0 || strlen(user.password) == 0 || strlen(user.accessibility) == 0) {

                        } else {
                            if(registration(&user, conn, res, row)) {
                        	    printf("Ok if\n");
                                send_message("Registration Successfull", user.username);
                                printf("Ok send message\n");
                                flag_login = login(&user, conn, res, row);
                                printf("\nFlag: %d\n", flag_login);
                        } else {
                              perror("ERROR: Registration Failed!!!");
                        }
                        
                        str_trim_lf(buff_out, strlen(buff_out));
                        
                        printf("%s -> %s\n", buff_out, user.username);
                        }
                    }
            } else if (receive == 0 || strcmp(buff_out, "exit") == 0) {
                    sprintf(buff_out, "\n%s has left\n", user->username);
                    printf("%s", buff_out);
                    //send_message(buff_out, cli->uid);
                    leave_flag = 1;
            } else {
                    printf("ERROR: -1\n");
                    leave_flag = 1;
            }
            bzero(buff_out, BUFFER_SZ);
        }

      /* Delete client from queue and yield thread */
      close(client->sockfd);
      //queue_remove(client->uid);
      free(client);
      //client_count--;
      pthread_detach(pthread_self());

      return NULL;
}

bool registration(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row) {
                        printf("Ok registration enter\n");
    res = send_query_insert(conn, res, row, u1);
                        printf("query fatt\n");
      if (res != NULL) {
        return true;
    } else {
        return false;
    }
}


bool login(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row) {
    res = send_query_select(conn, res, row, u1);
    close_connection(conn, res);

    if (res != NULL) {
        return true;
    } else {
        return false;
    }
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}


MYSQL *start_connection(MYSQL *conn) {
    char *server = "localhost";
    char *user = "root";
    char *password = "root"; /* set me first */
    char *database = "Accessibility";

    conn = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conn, server, user, password,
                                      database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    return conn;

}


MYSQL_RES *send_query_insert(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1) {
    char query_insert[1000];
                            printf("Ok send query user: %s, %s, %s \n", u1->username, u1->password, u1->accessibility);
    sprintf(query_insert, "insert into user(username, password, access) values ('%s', '%s', '%s')", u1->username, u1->password, u1->accessibility);
	printf("Dopo insert");

    if (mysql_query(conn, query_insert)) {
                            printf("query insert if ok\n");
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
                        printf("Ok query\n");
    res = mysql_use_result(conn);
                        printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA res\n");
    return res;
}


MYSQL_RES *send_query_select(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1) {
    char query_select[200];

    sprintf(query_select, "select * from user where username='%s'", u1->username);

    if (mysql_query(conn, query_select)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    printf("%s", query_select);

    /* output table name */
    printf("MySQL Tables in mysql database:\n");

    while ((row = mysql_fetch_row(res)) != NULL)
        printf("Username: %s Password: %s Accessibility: %s \n", row[0], row[1], row[2]);

    return res;
}


void close_connection(MYSQL *conn, MYSQL_RES *res) {
    mysql_free_result(res);
    mysql_close(conn);
}


void queue_add(client_t *client) {
        pthread_mutex_lock(&clients_mutex);
        printf("addiamo il client\n");
        for(int i=0; i < MAX_CLIENTS; ++i) {
        printf("nel for\n");
            if(!clients[i]) {
            printf("nell if client\n");
                    clients[i] = client;
                    printf("client messo\n");
                    break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);
}


void send_message(char *s, char* username) {
        pthread_mutex_lock(&clients_mutex);
        for(int i = 0; i < sizeof(clients)/sizeof(User); ++i) {
            if(clients[i]) {
                  User tmpuser = clients[i]->user;
                    if(tmpuser.username != username) {
                        if(write(clients[i]->sockfd, s, strlen(s)) < 0) {
                                perror("ERROR: write to descriptor failed");
                                break;
                           }
                    }
            }
        }

        pthread_mutex_unlock(&clients_mutex);
}
