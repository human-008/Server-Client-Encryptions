#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h> 

void* func(void* param){
    int client1 = *((int *)param);
    int client2 = *(((int *)param)+1);
    char msg[5000] = {0};
    int count = 0;
    while((count = recv(client1, msg, 5000, 0)) > 0){ // receives msg from one client
        msg[count] = '\0';
        send(client2, msg, count, 0); // sends it to the other client
    }   
    pthread_exit(0);
}

int main(int argc, char *argv[]){

    int port = atoi(argv[1]); // reads port number
    struct sockaddr_in server = {0};
    struct sockaddr_in client = {0};

    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < -1){
        printf("Unable to create Socket \n");
        return 1;
    }
    printf("Socket Created.... \n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    int binded = bind(socket_desc,(struct sockaddr *)&server,sizeof(server));

    if(binded < 0){
        printf("Socket Bind Failed \n");
        return 1;
    }

    listen(socket_desc, 2); // listening for two clients

    int len = sizeof(struct sockaddr_in);
    while(1){
        int c1 = accept(socket_desc, (struct sockaddr*) &client,(socklen_t*) &len); // accepting first client
        if(c1 < 0){
            printf("Error : not accepting.\n");
            return 1;
        }
        printf("Connection established with first client....\n");
        
        int c2 = accept(socket_desc, (struct sockaddr*) &client,(socklen_t*) &len); // accepting second client
        if(c2 < 0){
            printf("Error : not accepting.\n");
            return 1;
        }
        printf("Connection established with both clients.... Ready to run....\n"); 

        int clients[3] = {c1, c2, c1};

        pthread_t tid1, tid2;
        pthread_create(&tid1,NULL,func, clients);
        pthread_create(&tid2,NULL,func, clients+1);
        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);
        printf("Both the Clients Left......\n");
        close(c1);
        close(c2);
        printf("Waiting for new Clients......\n");
    }

    return 0;
}
