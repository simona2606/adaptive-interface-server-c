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
