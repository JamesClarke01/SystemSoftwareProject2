#include <stdio.h>
#include <string.h>    //for strlen..
#include <sys/socket.h>
#include <arpa/inet.h> //for inet_addr
#include <unistd.h>    //for write

#define PORT 8082
#define SERVER_IP "127.0.0.1"
#define FILE_BUFFER_SIZE 1024

int main(int argc , char *argv[])
{
    int SID;
    struct sockaddr_in server;
    char filePath[500];
    char serverMessage[500];
    char fileBuffer[FILE_BUFFER_SIZE] = {0};
    FILE* file;
     
    //Create socket
    SID = socket(AF_INET , SOCK_STREAM , 0);
    if (SID == -1){
        printf("Error creating socket.\n");
    } else {
    	printf("socket created.\n");
    } 
    
    // set sockaddr_in variables
    server.sin_port = htons(PORT); // Port to connect on
    server.sin_addr.s_addr = inet_addr(SERVER_IP); // Server IP
    server.sin_family = AF_INET; // IPV4 protocol
    
    //Connect to server
    if (connect(SID , (struct sockaddr *)&server , sizeof(server)) < 0) {
        printf("connect failed. Error\n");
        return 1;
    }
     
    printf("Connected to server ok!!\n");


    printf("\nEnter file path : ");
    scanf("%s" , filePath);
        
    //Reading file
    file = fopen(filePath, "rb");
    if (file == NULL) {
        printf("File path invalid.\n");
        return 1;
    }

    //Send file data line by line
    while (fgets(fileBuffer, FILE_BUFFER_SIZE, file) != NULL) {
        send(SID, fileBuffer, strlen(fileBuffer), 0);
    }    
     
    close(SID);
    return 0;
}
