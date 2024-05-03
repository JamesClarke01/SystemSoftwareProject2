#include <stdio.h>      // for IO
#include <stdlib.h>
#include <string.h>     //for strlen
#include <sys/socket.h> // for socket
#include <arpa/inet.h>  //for inet_addr
#include <unistd.h>     //for write

#define PORT 8081
#define FILE_BUFFER_SIZE 1024

int bindServerSocket(int *serverSocket) {
    struct sockaddr_in server;

    //Create socket
    *serverSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (*serverSocket == -1)
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
    if( bind(*serverSocket,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Bind issue.\n");
        return 1;
    } else {
    	printf("Bind Complete.\n");
    }
    return 0;
}

int acceptIncomingConnection(int* clientSocket, int* serverSocket) {
    struct sockaddr_in client;
    int connSize; // Size of struct 
    
    //Accept and incoming connection
    connSize = sizeof(struct sockaddr_in);
    //accept connection from an incoming client
    *clientSocket = accept(*serverSocket, (struct sockaddr *)&client, (socklen_t*)&connSize);
    if (*clientSocket < 0) {
        perror("Can't establish connection");
        return 1;
    } else {
    	printf("Connection from client accepted.\n");
    }
    return 0;
}

int receiveFile(int* clientSocket) {
    char fileName[FILE_BUFFER_SIZE];
    char buffer[BUFSIZ];
    FILE* file;
    int fileSize;
    ssize_t len;
    int remainData = 0;

    //Receive file name
    recv(*clientSocket, fileName, FILE_BUFFER_SIZE, 0);

    //Recevie file size
    recv(*clientSocket, buffer, BUFSIZ, 0);
    fileSize = atoi(buffer);

    //Open file for writing
    file = fopen(fileName, "wb");
    if (file == NULL) {
        printf("Error creating file");
        return 1;
    }  

    //Receive file
    remainData = fileSize;
    while ((remainData > 0) && ((len = recv(*clientSocket, buffer, BUFSIZ, 0)) > 0))
    {
        fwrite(buffer, sizeof(char), len, file);
        remainData -= len;
        fprintf(stdout, "Received %ld bytes, Remaning: %d bytes\n", len, remainData);
    }

}


int main(int argc , char *argv[])
{
    int serverSocket; // socket descriptor
    int clientSocket; // Client Socket
    int READSIZE;  // Size of sockaddr_in for client connection
    char message[500];
        
    if (bindServerSocket(&serverSocket) == 1) {
        return 1;
    }    
     
    //Listen for a conection
    listen(serverSocket,3); 
    printf("Waiting for incoming connection from Client...\n");

    if (acceptIncomingConnection(&clientSocket, &serverSocket) == 1) {
        return 1;
    }

    if (receiveFile(&clientSocket) == 1) {
        printf("Error receiving file");
        return 1;
    }
    
    send(clientSocket, "File Transferred Successfully\n", strlen("File Transferred Successfully\n"), 0);

    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(READSIZE == -1) {
        perror("read error");
    }
     
    return 0;
}
