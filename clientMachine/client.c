#include <stdio.h>
#include <string.h>    //for strlen..
#include <sys/socket.h>
#include <arpa/inet.h> //for inet_addr
#include <unistd.h>    //for write

//For files
#include <sys/stat.h> //For getting file size
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define PORT 8081
#define SERVER_IP "127.0.0.1"
#define FILE_BUFFER_SIZE 1024

//Extracts file name from path
const char* getFileName(const char* path) {
    const char* filename = strrchr(path, '/'); //Find last occurance of '/'

    if(!filename) //If / not found, return whole path
        return path;
    else //else return the file name
        return filename + 1;
}

int connectToServer(int* SID) {
    struct sockaddr_in server;

    //Create socket
    *SID = socket(AF_INET , SOCK_STREAM , 0);
    if (*SID == -1){
        printf("Error creating socket.\n");
    } else {
    	printf("socket created.\n");
    } 
    
    // set sockaddr_in variables
    server.sin_port = htons(PORT); // Port to connect on
    server.sin_addr.s_addr = inet_addr(SERVER_IP); // Server IP
    server.sin_family = AF_INET; // IPV4 protocol
    
    //Connect to server
    if (connect(*SID , (struct sockaddr *)&server , sizeof(server)) < 0) {
        printf("connect failed. Error\n");
        return 1;
    }

    printf("Connected to server ok.\n");
    return 0;
}

int sendFile(int* SID, char* filePath) {
    int fd;
    struct stat fileStat;
    char fileSize[256];
    int sentBytes = 0;
    long int offset;
    int remainData;
    
    //Reading file
    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        printf("File path invalid\n");
        return 1;
    }

    //Get file stats
    if (fstat(fd, &fileStat) < 0) {
        printf("Error getting file stats\n");
        return 1;
    } 

    //Send file name
    send(*SID, getFileName(filePath), FILE_BUFFER_SIZE, 0);
    
    //Send file size
    sprintf(fileSize, "%ld", fileStat.st_size); //Put file size in a buffer
    send(*SID, fileSize, sizeof(fileSize), 0); //send file size

    offset = 0;
    remainData = fileStat.st_size;

    //Sending file data 
    while (((sentBytes = sendfile(*SID, fd, &offset, BUFSIZ)) > 0) && (remainData > 0))
    {
        fprintf(stdout, "1. Server sent %d bytes, Offset: %ld, Remaining data: %d\n", sentBytes, offset, remainData);
        remainData -= sentBytes;
        fprintf(stdout, "2. Server sent %d bytes, Offset: %ld, Remaining data: %d\n", sentBytes, offset, remainData);
    }
    
    return 0;
}


int main(int argc , char *argv[])
{
    int SID;
    char filePath[500];
    char fileName[30];
    char serverMessage[500];
    char fileBuffer[FILE_BUFFER_SIZE] = {0};


    if (argc != 2) {
        printf("Invalid number of command line arguments entered\n");
        return 1;
    }

    if (connectToServer(&SID) == 1) {
        return 1;
    }

    strcpy(filePath, argv[1]);
    if (sendFile(&SID, filePath) == 1) {
        printf("Error sending file");
        return 1;
    }

    if( recv(SID , serverMessage , 500 , 0) < 0) {
        printf("Error receiving status message from server.\n");
        return 1;
    }

    printf("%s\n", serverMessage);
     
    close(SID);
    return 0;
}
