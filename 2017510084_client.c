#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048

int flag = 0;
int sock = 0;
//temporary variables
char nickname[32];
char command[100];
char* cmd;
char password[32];
char* pass;
char group_name[32];

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

void send_message() {
    char message[LENGTH] = {};
	char buffer[LENGTH] = {};
    char password[32];
    while(1) {
        fgets(message, LENGTH, stdin); //users message
        //string operations
        char temp[LENGTH];
        int len= strlen(message);
        if(message[len-1]=='\n'){
            message[len-1]='\0';
        }
        strcpy(temp,message);
        char* tok=strtok(message," "); 
        char command[10];
        strcpy(command,tok);
        tok = strtok(NULL, " ");
        char mes[LENGTH];
        while (tok!=NULL)
        {
            strcat(mes," ");
            strcat(mes,tok);
            tok = strtok(NULL, " ");
        }
        if (strcmp(temp, "-exit") == 0) {
            flag=1;
            printf("See you");
			break;
        }
        else if(strcmp(command, "-gcreate") == 0){
            printf("Please create a password : ");
            fgets(password, 30, stdin); //password
            char* ptr =strtok(mes,"+"); //nickname
            char* p=trim(ptr);
            strcpy(nickname,p);
            ptr=strtok(NULL," "); //groupname
            strcpy(group_name,ptr);
            sprintf(buffer, "%s;%s;%s;%s\n",command,nickname,group_name,password);
            send(sock, buffer, strlen(buffer), 0);
            
        }
        else if(strcmp(command, "-join") == 0){
            bzero(buffer, LENGTH);
            printf("Please enter the password : ");
            fgets(password, 30, stdin); //password
            char* p=trim(mes);
            strcpy(group_name,p);
            sprintf(buffer,"%s;%s;%s;%s", command,nickname,group_name,password);
            send(sock, buffer,strlen(buffer), 0);
        }
        else { //for send,whoami,exit group functions
            sprintf(buffer, "%s;%s;%s\n",command,mes,nickname);
            send(sock, buffer, strlen(buffer), 0);
        }
        bzero(temp,LENGTH);
        bzero(mes,LENGTH);
        bzero(message, LENGTH);
        bzero(buffer, LENGTH);
    }
}
void receive_message() {
    char message[LENGTH] = {};
    while(1){
        bzero(message, LENGTH);
        int receive = recv(sock, message, LENGTH, 0);
        if (receive > 0) {
            printf(">%s", message);
        }
        else{
            perror("Couldn't receive");
            break;
        }
      
    }
}
int main(int argc, char **argv){
    
    printf("COMMANDS\n");
    printf("At the beginning, 1-2-6 commands will work\n");
    printf("Please pay attentionn to command rules\n");
    printf("1-To create new group '-gcreate nickname+group name'\n");
    printf("2-Enter to specified group '-join group name'\n");
    printf("3-Exit group '-exit group name'\n");
    printf("4-To send a message '-send ..message..'\n");
    printf("5-Profile '-whoami'\n");
    printf("6-Exit program '-exit'\n");
    //at the beginning just first 2 command can work
    printf("Please enter command : ");

    fgets(command, 100, stdin); // at
    struct sockaddr_in server_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(3205);

    int con = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (con == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}
    
    
    char buffer[LENGTH] = {};
    char* ptr=strtok(command," ");
    bzero(buffer, LENGTH);
    if(strcmp(ptr, "-gcreate") == 0){
        printf("Please create a password : ");
        fgets(password, 30, stdin); //password
        strcpy(command,ptr); //command
        ptr=strtok(NULL,";");  //nickname+groupname
        ptr=strtok(ptr,"+"); //nickname
        strcpy(nickname,ptr);
        ptr=strtok(NULL," "); //groupname
        strcpy(group_name,ptr);
       
        int len= strlen(group_name);
        if(group_name[len-1]=='\n'){
            group_name[len-1]=0;
        }
        
        len= strlen(password);
        if(password[len-1]=='\n'){
            password[len-1]=0;
        }
        
        sprintf(buffer,"%s;%s;%s;%s", command,nickname,group_name,password);
        send(sock, buffer,strlen(buffer), 0);
    }
    else if(strcmp(ptr, "-join") == 0){
        printf("Please enter the password : ");
        fgets(password, 30, stdin); //password
        printf("Please enter a nickname : ");
        fgets(nickname, 30, stdin); //nickname
        ptr=strtok(NULL," "); //groupname
        strcpy(group_name,ptr);
        int len= strlen(group_name);
        if(group_name[len-1]=='\n'){
            group_name[len-1]=0;
        }
        
        len= strlen(password);
        if(password[len-1]=='\n'){
            password[len-1]=0;
        }
        len= strlen(group_name);
        if(group_name[len-1]=='\n'){
            group_name[len-1]=0;
        }
        
        len= strlen(nickname);
        if(nickname[len-1]=='\n'){
            nickname[len-1]=0;
        }
        sprintf(buffer,"%s;%s;%s;%s", command,nickname,group_name,password);
        send(sock, buffer,strlen(buffer), 0);
    }
    
    //created two threads, send and receive operetions will work simultaneously
    pthread_t send_thread;
    if(pthread_create(&send_thread, NULL, (void *) send_message, NULL) != 0){
		printf("Couldnt create a thread\n");
        return 1;
	}
    pthread_t recv_thread;
    if(pthread_create(&recv_thread, NULL, (void *) receive_message, NULL) != 0){
		printf("Couldnt create a thread\n");
		return 1;
	}
    while (1){
		if(flag==1){ //if user enters exit
			printf("See you..\n");
			break;
        }
    }
    close(sock); //at the end of the user operations
    
    return 0;
}