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

int main(int argc , char *argv[])
{
    int SID;
    struct sockaddr_in server;
    char filePath[500];
    char fileName[30];
    char serverMessage[500];
    char fileBuffer[FILE_BUFFER_SIZE] = {0};
    int fd;
    struct stat fileStat;
    char fileSize[256];
    int sent_bytes = 0;
    long int offset;
    int remain_data;

    if (argc != 2) {
        printf("Invalid number of command line arguments entered\n");
        return 1;
    }

    strcpy(filePath, argv[1]);
     
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
        
    //Reading file
    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        printf("File path invalid\n");
    }


    //Get file stats
    if (fstat(fd, &fileStat) < 0) {
        printf("Error getting file stats\n");
    } 

    //Send file name
    send(SID, getFileName(filePath), FILE_BUFFER_SIZE, 0);
    
    
    sprintf(fileSize, "%ld", fileStat.st_size); //Put file size in a buffer
    send(SID, fileSize, sizeof(fileSize), 0); //send file size


    offset = 0;
    remain_data = fileStat.st_size;

    /* Sending file data */
    while (((sent_bytes = sendfile(SID, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
    {
        fprintf(stdout, "1. Server sent %d bytes, Offset: %ld, Remaining data: %d\n", sent_bytes, offset, remain_data);
        remain_data -= sent_bytes;
        fprintf(stdout, "2. Server sent %d bytes, Offset: %ld, Remaining data: %d\n", sent_bytes, offset, remain_data);
    }


    if( recv(SID , serverMessage , 500 , 0) < 0)
    {
        printf("IO error.\n");
        //break;
    }

    printf("%s\n", serverMessage);
     
    close(SID);
    return 0;
}
