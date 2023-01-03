#include "client.h"
#include "utils.h"
#include "sql.h"

#define MAX_CLIENTS 5
#define BUFFER_SZ 2048

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
        char buff_out[BUFFER_SZ];
        User* user = (User *) malloc(sizeof(User));
        
        int leave_flag = 0;
        bool flag_login = false;
        
        MYSQL *conn;
    	MYSQL_RES *res;
    	MYSQL_ROW row;
        
        client_t *client = (client_t *)arg;
	char **arr = NULL;
	int i = 0;

        while(1) {
            if (leave_flag) {
                    break;
            }

            int receive = recv(client->sockfd, user, BUFFER_SZ, 0);
          
            if (receive > 0) {
            	
                    if(strlen(user->username) > 0) {
                    	int count = split(user->username, '-', &arr);
                    	for (i = 0; i < count; i++) {
                    		if (i == 0) {
                    			strcpy(client->user.username, arr[i]);
                    		} else if (i==1) {
                    			strcpy(client->user.password, arr[i]);
                    		} else if (i==2) {
                    			strcpy(client->user.accessibility, arr[i]);
                    		} else if (i==3) {
                    			strcpy(client->user.flag, arr[i]);
                    		}
                    		
                    		
                    	}
                    	
                    	
                        if(strlen(client->user.username) == 0 || strlen(client->user.password) == 0 || strlen(client->user.accessibility) == 0) {

                        } else {
                            if(client->user.flag[0] == 'r' && registration(&client->user, conn, res, row)) {
                            
                        	printf("\nSuccessfully registered %s\n", client->user.username);
                        	sprintf(buff_out, "\nSuccessfully registered %s\n", client->user.username);
                    		
                    		send_message(buff_out, client->uid);
                                
                                str_trim_lf(buff_out, strlen(buff_out));
                              
                            } else if(client->user.flag[0] == 'l') {
                        	flag_login = login(&client->user, conn, res, row);
                                
  				printf("\nSuccessfully logged with accessibility %s\n", client->user.accessibility);
                                sprintf(buff_out, "\nSuccessfully logged with accessibility %s\n", client->user.accessibility);
                                
                    		send_message(buff_out, client->uid);
                        	str_trim_lf(buff_out, strlen(buff_out));
                          } else {
                          	perror("ERROR: Registration Failed!!!");
                          }
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
            bzero(buff_out, BUFFER_SZ);
        }

	
      close(client->sockfd);
      queue_remove(client->uid);
      free(client);
      cli_count--;
      pthread_detach(pthread_self());

      return NULL;
}

/* Add clients to queue */
void queue_add(client_t *client) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(!clients[i]) {
            clients[i] = client;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to queue */
void queue_remove(int uid) {
    pthread_mutex_lock(&clients_mutex);

    for(int i = 0; i < MAX_CLIENTS; ++i) {
        if(clients[i]){
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
                if(write(clients[i]->sockfd, s, strlen(s)) < 0){
                    perror("ERROR: write to descriptor failed");
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}
