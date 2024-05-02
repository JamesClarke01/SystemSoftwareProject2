#include <stdio.h>      // for IO
#include <string.h>     //for strlen
#include <sys/socket.h> // for socket
#include <arpa/inet.h>  //for inet_addr
#include <unistd.h>     //for write

#define PORT 8082
#define FILE_BUFFER_SIZE 1024

int main(int argc , char *argv[])
{
    int serverSocket; // socket descriptor
    int clientSocket; // Client Socket
    int connSize; // Size of struct 
    int READSIZE;  // Size of sockaddr_in for client connection
    char fileBuffer[FILE_BUFFER_SIZE] = {0};
    char fileName[FILE_BUFFER_SIZE];
    FILE* file;

    struct sockaddr_in server , client;
    char message[500];
     
    //Create socket
    serverSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (serverSocket == -1)
    {
        printf("Could not create socket.\n");
    } else {
    	printf("Socket Successfully Created.\n");
    } 

    // set sockaddr_in variables
    server.sin_port = htons(PORT); // Set the prot for communication
    server.sin_family = AF_INET; // Use IPV4 protocol
    server.sin_addr.s_addr = INADDR_ANY; 
    // When INADDR_ANY is specified in the bind call, the  socket will  be bound to all local interfaces. 
     
    //Bind
    if( bind(serverSocket,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Bind issue.\n");
        return 1;
    } else {
    	printf("Bind Complete.\n");
    }
     
    //Listen for a conection
    listen(serverSocket,3); 
    //Accept and incoming connection
    printf("Waiting for incoming connection from Client>>\n");
    connSize = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    clientSocket = accept(serverSocket, (struct sockaddr *)&client, (socklen_t*)&connSize);
    if (clientSocket < 0) {
        perror("Can't establish connection");
        return 1;
    } else {
    	printf("Connection from client accepted.\n");
    }

    //Receive file name
    recv(clientSocket, fileName, FILE_BUFFER_SIZE, 0);

    //Open file for writing
    file = fopen(fileName, "wb");
    if (file == NULL) {
        printf("Error creating file");
        return 1;
    }  

    //Receive file line by line
    while (recv(clientSocket, fileBuffer, FILE_BUFFER_SIZE, 0) > 0) {
        fputs(fileBuffer, file);
    }

    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(READSIZE == -1) {
        perror("read error");
    }
     
    return 0;
}
