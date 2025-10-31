#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define ADMINNAME "admin"
#define ADMINPASS "admin"

#define ADMIN_LOCK_DB "db/adminLock"
#define COUNTERS_DB "db/counters"
#define CUSTOMERS_DB "db/customers"
#define EMPLOYEES_DB "db/employees"
#define FEEDBACKS_DB "db/feedbacks"
#define LOANAPPLICATIONS_DB "db/loanApplications"
#define TRANSACTIONS_DB "db/transactions"


#define BUFF_SIZE 4096

#include "Utils/fileLocking.h"
#include "Utils/menu.h"
#include "Structures/structures.h"
#include "Utils/getNextCounter.h"
#include "Utils/changePassword.h"
#include "Utils/logOut.h"
#include "Modules/admin.h"
#include "Modules/customer.h"
#include "Modules/employee.h"
#include "Modules/manager.h"

void handle_client(int nsd);

int main()
{
	int sd, nsd, portno = 5050;
	struct sockaddr_in serv, cli;
	socklen_t cli_len;
	char buf[BUFF_SIZE];

	// Create Socket
	sd = socket(AF_INET, SOCK_STREAM, 0);

	// Server Address Setup
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_port = htons(portno);

	//Allow Reusing Port immediately after program restart
	int opt = 1;
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// Bind Socket
	bind(sd, (struct sockaddr *)&serv, sizeof(serv));

	// Listen
	listen(sd, 5);
	printf("Concurrent Server Listening on port %d...\n", portno);

	while (1)
	{
		cli_len = sizeof(cli);
		nsd = accept(sd, (struct sockaddr *)&cli, &cli_len);
		printf("Connection Accepted.\n");
		
		if(nsd < 0)
		{
			perror("Accept Failed");
			continue;
		}
		else{
			if (fork() == 0)
			{
				close(sd);
				handle_client(nsd);
			}
			else
			{
				close(nsd);
			}
		}

	}

	close(sd);
	return 0;
}

void trim(char *str) {
    // Remove trailing newline and spaces
    str[strcspn(str, "\r\n")] = 0;

    // Remove leading spaces
    char *start = str;
    while (*start == ' ') start++;

    // Shift string left
    if (start != str)
        memmove(str, start, strlen(start) + 1);

    // Remove trailing spaces
    int end = strlen(str) - 1;
    while (end >= 0 && str[end] == ' ') str[end--] = 0;
}


void handle_client(int nsd)
{
	char readBuffer[BUFF_SIZE], writeBuffer[BUFF_SIZE];
	int readBytes, writeBytes, choice;

	while (1)
	{
		memset(writeBuffer, 0, BUFF_SIZE);
		writeBytes = write(nsd, MAIN_MENU, sizeof(MAIN_MENU));
		if(writeBytes < 0)
		{
			perror("Write to client failed");
		}
		else{

			memset(readBuffer, 0, BUFF_SIZE);
			readBytes = read(nsd, readBuffer, sizeof(readBuffer));
			trim(readBuffer);
			if(readBytes < 0)
			{
				perror("Read from client failed");
			}
			else if(readBytes == 0){
				printf("Client Disconnected.\n");			}
			else{
				choice = atoi(readBuffer);
				printf("Client Entered Choice: %d\n", choice);

				switch (choice)
				{
				case 1:
					signal(SIGINT, SIG_IGN); // Ignore SIGINT in child process
					customer_handler(nsd);
					signal(SIGINT, SIG_DFL); // Restore default behavior
					break;

				case 2:
					signal(SIGINT, SIG_IGN); // Ignore SIGINT in child process
					employee_handler(nsd);
					signal(SIGINT, SIG_DFL); // Restore default behavior
					break;

				case 3:
					signal(SIGINT, SIG_IGN); // Ignore SIGINT in child process
					manager_handler(nsd);
					signal(SIGINT, SIG_DFL); // Restore default behavior
					break;

				case 4:
					signal(SIGINT, SIG_IGN); // Ignore SIGINT in child process
					admin_handler(nsd);
					signal(SIGINT, SIG_DFL); // Restore default behavior
					break;

				case 5:
					close(nsd);
					exit(0);
					return;

				default:
					strcpy(writeBuffer, "Invalid choice. Please try again.\n");
					// Send error message to client
					write(nsd, writeBuffer, strlen(writeBuffer));
					// readBytes = read(nsd, readBuffer, sizeof(readBuffer));
					break;
				}
			}
		}
	}
}