#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFF_SIZE 4096

//handle if server is not available

void handle_connection(int sd);

int main()
{
    int sd, portno = 5050;
    struct sockaddr_in serv;
    char buffer[1024];

    // Socket Create
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Client Socket Created.\n");

    char ip[50];
    // printf("Enter Server IP Address : ");
    // scanf("%s", ip);

    // Server Address setup
    serv.sin_family = AF_INET;
    // serv.sin_addr.s_addr = inet_addr(ip);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_port = htons(portno);

    if (connect(sd, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
        perror("Connection to server failed");
        close(sd);
        _exit(1);
    }


    printf("Connected to Server %s:%d\n", ip, portno);
    handle_connection(sd);

    return 0;
}

void handle_connection(int sd)
{
    char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
    int readBytes, writeBytes;

    while (1)
    {
        memset(readBuffer, 0, BUFF_SIZE);
        readBytes = read(sd, readBuffer, sizeof(readBuffer));
        if(readBytes < 0)
        {
            perror("Read from server failed");
            return;
        }
        else if(readBytes == 0)
        {
            printf("Server closed the connection.\n");
            close(sd);
            return;
        }
        else
        {
            // Get user input
            memset(writeBuffer, 0, BUFF_SIZE);
            if(strstr(readBuffer, "Logging out...") != NULL){
                strcpy(writeBuffer, "");
                // close(sd);
                printf("Logged out successfully.\n");
                continue;
                // return;
            }
            else if(strstr(readBuffer, "Exiting...") != NULL){
                strcpy(writeBuffer, "");
                close(sd);
                printf("Client closed connection.\n");
                return;
            }
            else if(strchr(readBuffer, '^'))
            {
                strcpy(writeBuffer, "");
                close(sd);
                printf("Client exited successfully.\n");
                return;
            }
            else{
                printf("%s", readBuffer);
                // fgets() keeps the newline automatically.
                // It won’t leave garbage in stdin.
                // It won’t double \n.
                // while (1) {
                memset(writeBuffer, 0, BUFF_SIZE);
                fgets(writeBuffer, sizeof(writeBuffer), stdin);
                // writeBuffer[strcspn(writeBuffer, "\n")] = '\0';

                    // if (strlen(writeBuffer) == 0) {
                    //     printf("Input cannot be empty. Please enter a value: ");
                    //     continue;
                    // }
                    // break;
                // }
            }

            writeBytes = write(sd, writeBuffer, strlen(writeBuffer));
            if(writeBytes < 0)
            {
                perror("Write to server failed");
                break;
            }
        }
    }
    close(sd);
    _exit(0);
}