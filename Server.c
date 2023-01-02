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

#define MAX_CLIENTS 5
#define BUFFER_SZ 2048

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

typedef struct {
  char username[50];
  char password[50];
  char accessibility[20];
} User;

/* Client structure */
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

void str_overwrite_stdout();
void str_trim_lf (char* arr, int length);
void print_client_addr(struct sockaddr_in addr);

void *handle_client(void *arg);
void *handle_client_2(void *arg);

void send_message(char *s, int uid);
void queue_add(client_t *client);
void queue_remove(int uid);

int split (char *str, char c, char ***arr);

void start_socket();
bool registration(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row);
bool login(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row);


int main(int argc, char **argv) {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    if(argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = "0.0.0.0";
    int port = atoi(argv[1]);
    printf("Porta: %d", port);
    int option = 1;
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    //conn = start_connection(conn);

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    /* Ignore pipe signals */
    signal(SIGPIPE, SIG_IGN);

    if(setsockopt(listenfd,SOL_SOCKET, SO_REUSEADDR , &option, sizeof(option)))
    {
        perror("ERROR: setsockopt failed");
        return EXIT_FAILURE;
    }

    /* Bind */
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

  /* Listen */
    if (listen(listenfd, 10) < 0)
    {
        perror("ERROR: Socket listening failed");
        return EXIT_FAILURE;
    }

    printf("\n=== WELCOME TO THE SERVER ===\n");

    while(1)
    {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        // /* Check if max clients is reached */
        // if((cli_count + 1) == MAX_CLIENTS)
        // {
        //     printf("Max clients reached. Rejected: ");
        //     print_client_addr(cli_addr);
        //     printf(":%d\n", cli_addr.sin_port);
        //     close(connfd);
        //     continue;
        // }

        /* Client settings */
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid++;

        /* Add client to the queue and fork thread */
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client_2, (void*)cli);

        /* Reduce CPU usage */
        sleep(1);
    }

    return EXIT_SUCCESS;
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

MYSQL_RES *send_query_insert(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1) {
    char query_insert[200];
    
    
    printf("Ok send query user: %s, %s, %s \n", u1->username, u1->password, u1->accessibility);
    
    sprintf(query_insert, "insert into user(username, password, access) values ('%s', '%s', '%s')", u1->username, u1->password, u1->accessibility);
    
    if (mysql_query(conn, query_insert)) {
    	printf("query insert if ok\n");
	fprintf(stderr, "%s\n", mysql_error(conn));
	exit(1);
    }

    /*if (mysql_query(conn, ("insert into user values ('%s','%s','%s')",u1->username,u1->password,u1->accessibility))) {
        printf("query insert if ok\n");
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    printf("Ok query\n"); */
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
		printf("Username: %s Password: %s Accessibility: %s Surname: %s \n", row[0], row[1], row[2], row[3]);
	
	return res;
}

MYSQL *start_connection(MYSQL *conn) {
    char *server = "localhost";
    char *user = "root";
    char *password = "my-secret-password"; /* set me first */
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

void close_connection(MYSQL *conn, MYSQL_RES *res) {
    mysql_free_result(res);
    mysql_close(conn);
}

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
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

void print_client_addr(struct sockaddr_in addr) {
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

/* Add clients to queue */
void queue_add(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(!clients[i]){
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to queue */
void queue_remove(int uid) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid == uid){
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}



/* Send message to all clients except sender */
void send_message(char *s, int uid) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i]){
            if(clients[i]->uid != uid){
                if(write(clients[i]->sockfd, s, strlen(s)) < 0){
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

//void sendNumberClient(int count){
  //char c = count + '0';
  //send_message(&c, 0);
//}


int split (char *str, char c, char ***arr) {
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
}

void *handle_client_2(void *arg) {
        char buff_out[BUFFER_SZ];
        User* user = (User *) malloc(sizeof(User));
        
        int leave_flag = 0;
        bool flag_login = false;
        
        MYSQL *conn;
    	MYSQL_RES *res;
    	MYSQL_ROW row;
        printf("creiamo la struct\n");
        
        conn = start_connection(conn);
        
        client_t *client = (client_t *)arg;
	char **arr = NULL;
	int i = 0;
        
        printf("Prima if client\n");

        while(1) {
            if (leave_flag) {
                    break;
            }

            int receive = recv(client->sockfd, user, BUFFER_SZ, 0);
            printf("Dopo receive = %d\n", receive);
            if (receive > 0) {
            printf("Receive ok\n");
                    if(strlen(user->username) > 0) {
                    	int count = split(user->username, '-', &arr);
                    	for (i = 0; i < count; i++) {
                    		if (i == 0) {
                    			strcpy(client->user.username, arr[i]);
                    		} else if (i==1) {
                    			strcpy(client->user.password, arr[i]);
                    		} else if (i==2) {
                    			strcpy(client->user.accessibility, arr[i]);
                    		}
                    		printf("Risultato: %s\n", arr[i]);
                    		
                    	}
                    	
                    	printf("Username: %s", client->user.username);
                    	printf("Password: %s", client->user.password);
                    	printf("Access: %s", client->user.accessibility);
                    	
                        printf("Ok\n");
                        if(strlen(client->user.username) == 0 || strlen(client->user.password) == 0 || strlen(client->user.accessibility) == 0) {

                        } else {
                            if(registration(&client->user, conn, res, row)) {
                        	    printf("Ok if\n");
                                send_message("Registration Successfull", client->uid);
                                printf("Ok send message\n");
                                flag_login = login(&client->user, conn, res, row);
                                printf("\nFlag: %d\n", flag_login);
                        } else {
                              perror("ERROR: Registration Failed!!!");
                        }
                        
                        //str_trim_lf(buff_out, strlen(buff_out));
                        
                      // printf("%s -> %s\n", buff_out, user->username);
                        }
                    }
            } 
            else if (receive == 0 || strcmp(buff_out, "exit") == 0) {
                    sprintf(buff_out, "\n%s has left\n", client->user.username);
                    printf("%s", buff_out);
                    send_message(buff_out, client->uid);
                    leave_flag = 1;
            } 
            else {
                    printf("ERROR: -1\n");
                    leave_flag = 1;
            }
            //bzero(buff_out, BUFFER_SZ);
        }

      /* Delete client from queue and yield thread */
      close(client->sockfd);
      //queue_remove(client->uid);
      free(client);
      //client_count--;
      pthread_detach(pthread_self());

      return NULL;
}


/* Handle all communication with the client */
void *handle_client(void *arg)
{
    char buff_out[BUFFER_SZ];
    User* user = (User *) malloc(sizeof(User));
    int leave_flag = 0;
    bool flag_login = false;

    //cli_count++;
    client_t *cli = (client_t *)arg;
    //sendNumberClient(cli_count);

    // Name
    if(recv(cli->sockfd, user, 32, 0) <= 0){
        printf("Didn't enter the name.\n");
        leave_flag = 1;
    } else {
        strcpy(cli->user.username, user->username);
        sprintf(buff_out, "%s has joined\n", cli->user.username);
        printf("%s", buff_out);
        send_message(buff_out, cli->uid);
    }

    bzero(buff_out, BUFFER_SZ);

    while(1) {
        if (leave_flag) {
            break;
        }

        int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
        printf("dopo receive = %d\n", receive);
        if (receive > 0){
            if(strlen(buff_out) > 0) {
                send_message(buff_out, cli->uid);

                str_trim_lf(buff_out, strlen(buff_out));
                printf("%s -> %s\n", buff_out, cli->user.username);
            }
        } else if (receive == 0 || strcmp(buff_out, "exit") == 0){
            sprintf(buff_out, "\n%s has left\n", cli->user.username);
            printf("%s", buff_out);
            send_message(buff_out, cli->uid);
            leave_flag = 1;
        } else {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buff_out, BUFFER_SZ);
    }

  /* Delete client from queue and yield thread */
  close(cli->sockfd);
  queue_remove(cli->uid);
  free(cli);
  //cli_count--;
  pthread_detach(pthread_self());

  return NULL;
}
