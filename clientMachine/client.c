#include <stdio.h>
#include <string.h>    //for strlen..
#include <sys/socket.h>
#include <arpa/inet.h> //for inet_addr
#include <unistd.h>    //for write
#include <grp.h>
#include <unistd.h>
#include <stdlib.h> //Malloc & Free

//For files
#include <sys/stat.h> //For getting file size
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define PORT 8081
#define SERVER_IP "127.0.0.1"
#define FILE_BUFFER_SIZE 1024

#define SALES_GROUP_NAME "Sales"
#define DISTRIBUTION_GROUP_NAME "Distribution"
#define MANUFACTURING_GROUP_NAME "Manufacturing"

enum Department {
    SALES,
    DISTRIBUTION,
    MANUFACTURING
};

//Extracts file name from path
const char* getFileName(const char* path) {
    const char* filename = strrchr(path, '/'); //Find last occurance of '/'

    if(!filename) //If / not found, return whole path
        return path;
    else //else return the file name
        return filename + 1;
}

//Returns the department the current user belongs to by checking user group
enum Department getUserDepartment() {
    
    enum Department userDepartment = -1;
    uid_t uid = getuid(); //Get the current user's ID

    //Get the number of groups the user belongs to
    int ngroups = getgroups(0, NULL);
    if (ngroups < 0) {
        perror("getgroups");
        return 1;
    }

    //Allocate memory for the array of group IDs
    gid_t *groups = malloc(ngroups * sizeof(gid_t));
    if (groups == NULL) {
        perror("malloc");
        return 1;
    }

    //Get Groups of user
    if (getgroups(ngroups, groups) < 0) {
        perror("getgroups");
        free(groups);
        return 1;
    }


    for (int i = 0; i < ngroups; i++) {
        struct group *grp = getgrgid(groups[i]);
        if (grp != NULL) {
            if(strcmp(grp->gr_name, SALES_GROUP_NAME) == 0) {
                userDepartment = SALES;
            } else if (strcmp(grp->gr_name, DISTRIBUTION_GROUP_NAME) == 0) {
                userDepartment = DISTRIBUTION;
            } else if (strcmp(grp->gr_name, MANUFACTURING_GROUP_NAME) == 0) {
                userDepartment = MANUFACTURING;
            }
        }
    }

    free(groups);
    return userDepartment;
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
    char serverMessage[500];
    char userDepartment[2];
    char fileBuffer[FILE_BUFFER_SIZE] = {0};
    

    if (argc != 2) {
        printf("Invalid number of command line arguments entered\n");
        return 1;
    }

    if (connectToServer(&SID) == 1) {
        return 1;
    }

    //Send user department
    if (getUserDepartment() == -1) {
        printf("User does not belong to a valid department\n");
        return 1;
    }
    sprintf(userDepartment, "%d", getUserDepartment());
    send(SID, userDepartment, 2, 0);
    
    strcpy(filePath, argv[1]);
    if (sendFile(&SID, filePath) == 1) {
        printf("Error sending file\n");
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
