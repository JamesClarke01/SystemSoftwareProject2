#include <stdio.h>      // for IO
#include <stdlib.h>
#include <string.h>     //for strlen
#include <sys/socket.h> // for socket
#include <arpa/inet.h>  //for inet_addr
#include <unistd.h>     //for write
#include <pthread.h>

#include "shared.h"

#define SALES_DIR "Sales"
#define DISTRIBUTION_DIR "Distribution"
#define MANUFACTURING_DIR "Manufacturing"


enum Department {
    SALES,
    DISTRIBUTION,
    MANUFACTURING
};

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

void* handleClientTransfer(void* clientSocketPtr) {
    char fileName[FILE_BUFFER_SIZE];
    char buffer[BUFSIZ];
    FILE* file;
    int fileSize;
    ssize_t len;
    int remainData = 0;
    char department[2];
    enum Department userDepartment;
    char filePath[100];
    int READSIZE;  // Size of sockaddr_in for client connection
    int chownRes;

    int clientSocket = *((int*)clientSocketPtr);

    //Receive department and set directory
    if(recv(clientSocket , department , 500 , 0) < 0) {
        printf("Error receiving status message from server.\n");
    }
    userDepartment = atoi(department);

    //Receive file name
    recv(clientSocket, fileName, FILE_BUFFER_SIZE, 0);

    //Recevie file size
    recv(clientSocket, buffer, BUFSIZ, 0);
    fileSize = atoi(buffer);    

    //Make file path
    switch(userDepartment) {
        case SALES:
            strcpy(filePath, SALES_DIR);
            break;    
        case DISTRIBUTION:
            strcpy(filePath, DISTRIBUTION_DIR);
            break;   
        case MANUFACTURING:
            strcpy(filePath, MANUFACTURING_DIR);
            break;  
    }

    strcat(filePath, "/");
    strcat(filePath, fileName);

    //Open file for writing
    file = fopen(filePath, "wb");
    if (file == NULL) {
        printf("Error creating file\n");
    }  

    //Receive file
    remainData = fileSize;
    while ((remainData > 0) && ((len = recv(clientSocket, buffer, BUFSIZ, 0)) > 0))
    {
        fwrite(buffer, sizeof(char), len, file);
        remainData -= len;
        fprintf(stdout, "Received %ld bytes, Remaning: %d bytes\n", len, remainData);
    }

    sleep(15);

    send(clientSocket, "File Transferred Successfully\n", strlen("File Transferred Successfully\n"), 0);

    //Set file owner
    switch(userDepartment) {
        case SALES:
            chownRes = chown(filePath, SALES_UID, SALES_GID);
            break;    
        case DISTRIBUTION:
            chownRes = chown(filePath, DISTRIBUTION_UID, DISTRIBUTION_GID);
            break;   
        case MANUFACTURING:
            chownRes = chown(filePath, MANUFACTURING_UID, MANUFACTURING_GID);
            break;  
    }
    if(chownRes != 0) {
        printf("Error changing file ownership\n");
    }

    if(READSIZE == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if(READSIZE == -1) {
        perror("read error");
    }

    fclose(file);
    close(clientSocket);

    return NULL;
}

int main(int argc , char *argv[])
{
    int serverSocket; // socket descriptor
    int clientSocket; // Client Socket
    char message[500];
    struct sockaddr_in client;
    int connSize; // Size of struct 
    pthread_t threadID;
        
    if (bindServerSocket(&serverSocket) == 1) {
        return 1;
    }    
     
    while(1) {
        //Listen for a conection
        listen(serverSocket,3); 
        printf("Waiting for incoming connection from Client...\n");
    
        //Accept and incoming connection
        connSize = sizeof(struct sockaddr_in);

        //accept connection from an incoming client
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&client, (socklen_t*)&connSize)) < 0){
            printf("Can't establish connection\n");
            continue;
        }
        
        // Create a thread to handle the new connection
        if (pthread_create(&threadID, NULL, handleClientTransfer, (void *)&clientSocket) != 0) {
            perror("pthread_create");
            close(clientSocket); // Close the socket if thread creation fails
            continue; // Skip to the next iteration
        }

        pthread_detach(threadID);
        
    }
    return 0;
}
