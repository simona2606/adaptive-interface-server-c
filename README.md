# Adaptive Interface News Server 🔶

## Compilation guide

To get the server object code in C, it is necessary to compile the files in the directory:
```gcc -o app.out ./server/main.c ./server/client.c ./server/sql.c ./server/utils.c -lmysqlclient```

Once compiled, we can launch into execution with the command:
```./app.out <Port_number>```

Specifying the port number on which the server listens. By client setting, we recommend using port 5003.
To simplify this, we have created a bash file that contains these two commands listed above. In this way, to execute the code it will be enough to type that command:
```./build.sh```

The first time this command will give an error if you do not assign permissions to that file. You must use the ***chmod*** command to do this.

## Server C 
For each new connection, the server creates a pthread to handle client requests, which is dropped when the client closes the connection. The addition and removal of clients, as well as the exchange of messages between client and server, are handled through a mutex semaphore, so that race-conditions do not occur.
```
void queue_add(client_t *client) { 
    pthread_mutex_lock(&clients_mutex); 
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(!clients[i]) { 
            clients[i] = client; break;
        } 
    }
    pthread_mutex_unlock(&clients_mutex); 
}

void queue_remove(int uid) { 
    pthread_mutex_lock(&clients_mutex);
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[I]) { 
            if(clients[i]->uid == uid) {
                clients[i] = NULL;
                break; 
            }
         } 
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *s, int uid) { 
    pthread_mutex_lock(&clients_mutex); 
    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i]) { 
            if(clients[i]->uid == uid) {
                if(write(clients[i]->sockfd, s, strlen(s)) < 0) { 
                    perror("ERROR: write to descriptor failed"); 
                    break;
                } 
              }
         } 
    }
    pthread_mutex_unlock(&clients_mutex); 
}
```

Regarding the insertion and retrieval of user data within the database, special functions containing SQL queries have been created. Before each query it is necessary to initiate a connection with the database and at the end of each query it is necessary to close this connection to avoid "range-out-of-index" problems.
Below is an example of a user login function:
```
MYSQL *start_connection(MYSQL *conn) { 
    char *server = "localhost";
    char *user = "root";
    char *password = "my-secret-password"; 
    char *database = "Accessibility";
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) { 
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    return conn; 
}

void close_connection(MYSQL *conn, MYSQL_RES *res) { 
    mysql_free_result(res);
    mysql_close(conn);
}
```

```
User *send_query_select(MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row, User *u1) { 
    char query_select[200];
    sprintf(query_select, "select access from user where username='%s'", u1->username);
    if (mysql_query(conn, query_select)) { 
        fprintf(stderr, "%s\n", mysql_error(conn)); 
        exit(1);
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
        strcpy(u1->accessibility, row[0]); 
    }
    return u1; 
}

bool login(User *u1, MYSQL *conn, MYSQL_RES *res, MYSQL_ROW row) { 
    conn = start_connection(conn);
    u1 = send_query_select(conn, res, row, u1);
    close_connection(conn, res);
    if (u1 != NULL) { 
        return true;
    } else {
        return false;
    } 
}
```
