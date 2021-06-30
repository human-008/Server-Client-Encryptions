#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/pem.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int const padding = RSA_PKCS1_PADDING;
int socket_desc=0; // sockets description


void* writing(void* param); // function to read from console


int main(int argc,char *argv[]){



    int port; // reading port no
    char* ip_addr; // reading ip
    ip_addr = argv[1];
    port = atoi(argv[2]);

    struct sockaddr_in server = {0};

    if((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Socket can't be created");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr(ip_addr); //Local Host//change
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    int con=connect(socket_desc,(struct sockaddr *)&server,sizeof(struct sockaddr_in));

    if(con==-1){
        printf("\n connection to socket failed \n");
        return 0;
    }
    printf("Sucessfully conected with server\n");

    pthread_t t;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&t, &attr, writing, argv[4]);

    char msg[5000];
    unsigned char encrypted[100000];



    FILE* keyfile_decrypt=fopen(argv[3], "rb");

    RSA *rsa = RSA_new();
    rsa = PEM_read_RSAPrivateKey(keyfile_decrypt, &rsa, NULL, NULL);
    if(rsa == NULL){
        printf( "Failed to create RSA for decryption.\n");
        return 0;
    }
    // client thread always ready to receive input from console
    int len=0;
    while((len = recv(socket_desc,encrypted,10000,0)) > 0) {
        encrypted[len] = '\0';
        int res = RSA_private_decrypt(len, encrypted, (unsigned char *)msg, rsa, padding);
        msg[res] = '\0';
        if(strcmp(msg, "exit") == 0)
            break;

        printf("Encrpyted Text : %s\nOriginal Message : %s\n", encrypted, msg);
        fflush(stdout);
    }
    close(socket_desc);
    fclose(keyfile_decrypt);
    shutdown(socket_desc, 0);
    shutdown(socket_desc, 1);
    shutdown(socket_desc, 2);
    exit(0);

    
}

void* writing(void* param)
{
    
    char msg[4096];
    unsigned char encrypted[10000];


    FILE* keyfile_encrypt = fopen((char*)param, "rb"); // reading other clients public key for decryption

    RSA *rsa = RSA_new();
    rsa = PEM_read_RSA_PUBKEY(keyfile_encrypt, &rsa, NULL, NULL);
    if(rsa == NULL){
        printf( "Failed to create RSA for Encryption.\n");
        return 0;
    }


    //ready to read a message from console
    while(1) {
        fgets(msg,2000,stdin);
        msg[strlen(msg)-1]='\0';
        int res = RSA_public_encrypt(strlen(msg), (unsigned char*)msg, encrypted, rsa, padding);
        send(socket_desc, encrypted, res, 0);

        if(strcmp(msg, "exit") == 0)
            break;
    }

    close(socket_desc);
    fclose(keyfile_encrypt);
    shutdown(socket_desc, 0);
    shutdown(socket_desc, 1);
    shutdown(socket_desc, 2);
    exit(0);
}
