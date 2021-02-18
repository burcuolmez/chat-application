#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>


#define BUFFER_SIZE 2048 
#define MAX_CLIENTS 100 //to define an clients array
static int clientCounter=0; //to keep track of clients count
static int id=0; 

typedef struct{ //to keep clients information 
	struct sockaddr_in address;
	int sock;
	int id;
	char nickname[32];
    char group_name[32];
    char password[32];
} client;

client *clients[MAX_CLIENTS]; //this array keeps all the clients information all together
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void send_message(char *message, int id, char *group){ //to transmit message to group members 
	pthread_mutex_lock(&mutex); //used mutex to provide confusion with threads
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
            //this will write the message to other group members page and checks group name 
			if(clients[i]->id != id && strcmp(clients[i]->group_name,group)==0 && strcmp(clients[i]->group_name," ")!=0 ){ 
				if(write(clients[i]->sock, message, strlen(message)) < 0){
					perror("Couldn't write to clients page");
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&mutex); //unlocked the mutex so other thread can work here now
}

	
//to get rid of spaces
char* trim(char* str) 
{ 
    static char str1[99]; 
    int count = 0, j, k; 
    while (str[count] == ' ') { 
        count++; 
    } 
    for (j = count, k = 0; 
         str[j] != '\0'; j++, k++) { 
        str1[k] = str[j]; 
    } 
    str1[k] = '\0'; 
  
    return str1; 
}

//all client operations in serverside  
void *handle_connection(void *arg){
    char buff_out[BUFFER_SIZE];
    char buf2[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char nick[BUFFER_SIZE];
	int flag = 0; //to keep track users condition
    clientCounter++; 
	client *cli = (client *)arg; //client has created in server
    bzero(buff_out, BUFFER_SIZE);
    if(recv(cli->sock, buff_out, BUFFER_SIZE, 0) <= 0){ //recv functions output must be greater than 0 
		printf("Didn't enter the command.\n");
		flag = 1;
	}
    else{
        //first operations are gcreate and join, when user gets in to the system user can do only this two operation
        char* token=strtok(buff_out,";");
        if(strcmp(token,"-gcreate")==0){
            token = strtok(NULL, ";");
            strcpy(cli->nickname, token);
            token = strtok(NULL, ";");
            strcpy(cli->group_name, token);
            token = strtok(NULL, ";");
            strcpy(cli->password, token);
            bzero(buff_out, BUFFER_SIZE);
            sprintf(buff_out, "%s has created group %s password: %s \n", cli->nickname,cli->group_name,cli->password);
            printf("%s", buff_out);
            bzero(buff_out, BUFFER_SIZE);
        }
        else if(strcmp(token,"-join")==0){
            token = strtok(NULL, ";");
            strcpy(cli->nickname, token);
            token = strtok(NULL, ";");
            strcpy(cli->group_name, token);
            token = strtok(NULL, ";");
            strcpy(cli->password, token);
            bzero(buff_out, BUFFER_SIZE);
            //password check
            int f=0;
            for(int i=0; i<MAX_CLIENTS; ++i){
                if(clients[i]){
                    if(clients[i]->id !=cli->id && strcmp(clients[i]->group_name,cli->group_name)==0 && strcmp(clients[i]->group_name," ")!=0 && strcmp(clients[i]->password,cli->password)==0){
                        f=1;
                        break;
                    }
                }
            }
            if(f==1){
                sprintf(buff_out, "%s has joined group %s password: %s \n", cli->nickname,cli->group_name,cli->password);
                printf("%s", buff_out);
                bzero(buff_out, BUFFER_SIZE);
            }
            else{
                strcpy(cli->group_name, " ");
                strcpy(cli->password, " ");
                strcpy(buff_out,"Group name or password is wrong please try again with whole command..\n");
                printf("%s",buff_out);
                write(cli->sock,buff_out,BUFFER_SIZE);
                bzero(buff_out, BUFFER_SIZE);
            }
            
        }
        else{
            strcpy(buff_out,"Wrong arguements..\n");
            printf("%s\n",buff_out);
            for(int i=0; i<MAX_CLIENTS; ++i){
		        if(clients[i]){
			        if(clients[i]->id == cli->id){
				        if(write(clients[i]->sock, buff_out,BUFFER_SIZE ) < 0){
					        perror("Couldn't write to clients page");
					        break;
				        }
			        }
		        }
	        }
        }
        
        
     

        while(1){
            if (flag) {
                break;
            }
            int receive = recv(cli->sock, buff_out, BUFFER_SIZE, 0);
            if (receive > 0){
                strcpy(buf2,buff_out);
                char* token=strtok(buf2,";");
                if(strcmp(token,"-gcreate")==0){
                    if(cli->group_name!=NULL && strcmp(cli->group_name," ")!=0){
                        bzero(buff_out,BUFFER_SIZE);
                        sprintf(buff_out, "%s has left the group %s\n", cli->nickname,cli->group_name);
                        printf("%s", buff_out);
                        send_message(buff_out, cli->id,cli->group_name); 
                    }
                    //string operations
                    token = strtok(NULL, ";");
                    strcpy(cli->nickname, token);
                    token = strtok(NULL, ";");
                    strcpy(cli->group_name, token);
                    token = strtok(NULL, ";");
                    strcpy(cli->password, token);
                    bzero(buf2, BUFFER_SIZE);
                    sprintf(buf2, "%s has created group %s password: %s \n", cli->nickname,cli->group_name,cli->password);
                    printf("%s", buf2);
                    bzero(buf2, BUFFER_SIZE);
                }
                else if(strcmp(token,"-join")==0){
                    //string operations
                    token = strtok(NULL, ";");
                    strcpy(cli->nickname, token);
                    token = strtok(NULL, ";");
                    strcpy(cli->group_name, token);
                    token = strtok(NULL, ";");
                    strcpy(cli->password, token);
                    bzero(buf2, BUFFER_SIZE);
                    //checks password
                    int f=0;
                    for(int i=0; i<MAX_CLIENTS; ++i){
                        if(clients[i]){
                            if(clients[i]->id !=cli->id && strcmp(clients[i]->group_name,cli->group_name)==0 && strcmp(clients[i]->group_name," ")!=0 && strcmp(clients[i]->password,cli->password)==0){
                                f=1;
                                break;
                            }
                        }
                    }
                    if(f==1){
                        sprintf(buff_out, "%s has joined group %s password: %s \n", cli->nickname,cli->group_name,cli->password);
                        printf("%s", buff_out);
                        bzero(buff_out, BUFFER_SIZE);
                    }
                    else{
                        strcpy(cli->group_name, " ");
                        strcpy(cli->password, " ");
                        strcpy(buff_out,"Group name or password is wrong please try again with whole command..\n");
                        printf("%s",buff_out);
                        write(cli->sock,buff_out,BUFFER_SIZE);
                        bzero(buff_out, BUFFER_SIZE);
                    }
                }
                else if(strcmp(token,"-exit")==0){ //exit group 
                    token = strtok(NULL, ";");
                    char* p;
                    p=trim(token);
                    if(strcmp(p,cli->group_name)==0){
                        bzero(buff_out,BUFFER_SIZE);
                        sprintf(buff_out, "%s has left\n", cli->nickname);
                        printf("%s", buff_out);
                        send_message(buff_out, cli->id,cli->group_name);
                        strcpy(cli->group_name," "); //changed group name

                    }
                }
                else if(strcmp(token,"-send")==0){
                    char* sp=strtok(buff_out,";"); //includes send
                    //printf("%s\n",sp);
                    sp = strtok(NULL, ";");
                    strcpy(message,sp);
                    //printf("message: %s \n",message);
                    sp = strtok(NULL, ";");
                    strcpy(nick,sp);
                    //printf("nick: %s \n",nick);
                    int len = strlen(nick);
                    if(nick[len-1]=='\n'){
                        nick[len-1]='\0';
                    }
                    strcat(nick,":");
                    strcat(nick,message);
                    strcat(nick," \n");
                    printf("%s\n",nick);
                    send_message(nick, cli->id, cli->group_name);
                }
                else if(strcmp(token,"-whoami")==0){
                    sprintf(buff_out, "You are: %s \n", cli->nickname);
                    printf("%s", buff_out);
                    	for(int i=0; i<MAX_CLIENTS; ++i){
		                    if(clients[i]){
			                    if(clients[i]->id == cli->id){
				                    if(write(clients[i]->sock, buff_out,BUFFER_SIZE ) < 0){
					                    perror("Couldn't write to clients page");
					                    break;
				                    }
			                    }
		                    }
	                    }
                }
                else{
                    strcpy(buff_out,"Wrong arguements..\n");
                    printf("%s\n",buff_out);
                   	for(int i=0; i<MAX_CLIENTS; ++i){
		                    if(clients[i]){
			                    if(clients[i]->id == cli->id){
				                    if(write(clients[i]->sock, buff_out,BUFFER_SIZE ) < 0){
					                    perror("Couldn't write to clients page");
					                    break;
				                    }
			                    }
		                    }
	                    }
                }
            }
            else if(receive == 0 || strcmp(buff_out, "-exit") == 0){ //exits program or receives nothing
                sprintf(buff_out, "%s has left\n", cli->nickname);
                printf("%s", buff_out);
                send_message(buff_out, cli->id,cli->group_name);
                strcpy(cli->group_name," ");
                flag=1;
            }
            else {
                printf("Couldnt receive message from user\n");
                flag = 1;
            }
            //to clean inside of variables to avoid re-write
            bzero(buff_out, BUFFER_SIZE);
            bzero(buf2,BUFFER_SIZE);
            bzero(message,BUFFER_SIZE);
            bzero(nick,BUFFER_SIZE);
            
        }
    close(cli->sock); //client operations are done
    clients[id] = NULL; //to get new clients
    free(cli);
    clientCounter--;
    pthread_detach(pthread_self()); //to free thread

	return NULL;
        
    }
}

int main(int argc, char **argv){
    int sock_desc = 0, sock_acc = 0;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t tid;

    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(3205);
    //binding operations
    if(bind(sock_desc, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Socket binding failed");
    return 1;
    }
    //listen operations
    if (listen(sock_desc, 10) < 0) {
    perror("Socket listening failed");
    return 1;
	}

    while(1){
        socklen_t clilen = sizeof(cli_addr);
        sock_acc = accept(sock_desc, (struct sockaddr*)&cli_addr, &clilen);
        if((clientCounter + 1) == MAX_CLIENTS){
			printf("App is full ");
			close(sock_acc); //server cant accept more clients
			continue;
		}
        client *cli = (client *)malloc(sizeof(client));
		cli->address = cli_addr;
		cli->sock = sock_acc;
		cli->id = id++;
        clients[id] = cli;
        pthread_create(&tid, NULL, &handle_connection, (void*)cli); //all the client operations happens in here
        sleep(1);
    }
    return 0;
}
