#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>

#define ADMINNAME "admin"
#define ADMINPASS "admin"

#define COUNTERS_DB "db/counters"
#define CUSTOMERS_DB "db/customers"
#define EMPLOYEES_DB "db/employees"
#define FEEDBACKS_DB "db/feedbacks"
#define LOANAPPLICATIONS_DB "db/loanApplications"
#define TRANSACTIONS_DB "db/transactions"


#define BUFF_SIZE 4096

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
					customer_handler(nsd);
					break;

				case 2:
					employee_handler(nsd);
					break;

				case 3:
					manager_handler(nsd);
					break;

				case 4:
					admin_handler(nsd);
					break;

				case 5:
					exit_client(nsd);
					return;

				default:
					printf("Invalid Choice from Client. Please Try Again. \n");
					break;
				}
			}
		}
	}
}