#include "server.h"

void throwout(int fdSocket, int type) {

	char person1[100];
	char person2[100];
	strcpy(person1, "You are THROWN OUT by the server");
	strcpy(person2, "You partner has been THROWN OUT");

	strcpy(person1, "You are ENDED by the server");

	if (pairedPartners[fdSocket] != fdSocket && fdSocket > -1) {

		struct packet throwPacket;
		struct packet throwPacketClient;
		strcpy(throwPacket.command, "SERVER_THROWOUT");
		strcpy(throwPacketClient.command, "SERVER_THROWOUT");

		strcpy(throwPacket.message, person1);
		strcpy(throwPacketClient.message, person2);

		sendDataPacket(fdSocket, &throwPacket);
		sendDataPacket(pairedPartners[fdSocket], &throwPacketClient);

		removeChats(fdSocket, pairedPartners[fdSocket]);
	} else if (pairedPartners[fdSocket] == fdSocket) {
		struct packet throw_packet;
		strcpy(throw_packet.command, "SERVER_THROWOUT");

		strcpy(throw_packet.message, person1);
		sendDataPacket(fdSocket, &throw_packet);
		removeChat(fdSocket, 1);
	}
}

void clear_var(int c) {
	pairedPartners[c] = -2;
	blockedUsers[c] = 0;
	noOfBytesUsed[c] = 0;
	memset(&clientUser[c], 0, COM_SIZE * sizeof(clientUser[0]));
}

void end() {
	int c;
	for (c = 0; c < ACTIVE_LIST; c++) {
		if (pairedPartners[c] > -2) {
			throwout(c, 3);
		}
		clear_var(c);
	}
	startServer = 0;
}

void print_server_stats() {
	int c;
	int num_chat_queue = 0;
	int num_chatting = 0;
	int num_blocked = 0;
	FILE *fp;

	fp = fopen("serverstatus.txt", "w");
	for (c = 0; c < ACTIVE_LIST; c++) {
		if (pairedPartners[c] == -1 || pairedPartners[c] == c)
			num_chat_queue++;
		else if (pairedPartners[c] != c && pairedPartners[c] != -2) {
			num_chatting++;
		}
		if (blockedUsers[c] > 0)
			num_blocked++;
	}
	printf("*******************************************\n");
	printf("***************SERVER STATS****************\n");
	printf("*******************************************\n");

	fprintf(fp, "*******************************************\n");
	fprintf(fp, "***************SERVER STATS****************\n");
	fprintf(fp, "*******************************************\n");

	printf("\nClients on chat queue: %d\n", num_chat_queue);
	fprintf(fp, "\nClients on chat queue: %d\n", num_chat_queue);
	printf("____________________________________________\n");
	fprintf(fp, "____________________________________________\n");
	printf("Client username\t|\tSocket Number\n");
	fprintf(fp, "Client username\t|\tSocket Number\n");
	printf("____________________________________________\n");
	fprintf(fp, "____________________________________________\n");

	for (c = 0; c < ACTIVE_LIST; c++) {
		if (pairedPartners[c] == -1) {
			printf("%s\t\t|\t\t%d\n", "<<no username set yet>>", c);
			fprintf(fp, "%s\t\t|\t\t%d\n", "<<no username set yet>>", c);
		} else if (pairedPartners[c] == c) {
			printf(" %s \t | %d \n", clientUser[c], c);
			fprintf(fp, " %s \t | %d \n", clientUser[c], c);
		}
	}

	printf("\nClient Chatting: %d\n", num_chatting);
	fprintf(fp, "\nClient Chatting: %d\n", num_chatting);
	printf(
			"______________________________________________________________________\n");
	fprintf(fp,
			"______________________________________________________________________\n");
	printf(
			"First Client username | First Client Socket Number -> Second Client username | Second Client Socket Number || Bytes Used\n");
	fprintf(fp,
			"First Client username | First Client Socket Number -> Second Client username|Second        Client Socket Number || Bytes Used\n");
	printf(
			"_______________________________________________________________________\n");
	fprintf(fp,
			"_______________________________________________________________________\n");

	for (c = 0; c < ACTIVE_LIST; c++) {
		if (pairedPartners[c] != -1 && pairedPartners[c] != c
				&& pairedPartners[c] != -2) {

			printf("%s\t\t|\t\t %d\t\t -> \t\t%s\t\t|\t\t%d\t\t||\t%d\n",
					clientUser[c], c, clientUser[pairedPartners[c]],
					pairedPartners[c], noOfBytesUsed[c]);
			fprintf(fp, "%s\t\t|\t\t %d\t\t -> \t\t%s\t\t|\t\t%d\t\t||\t%d\n",
					clientUser[c], c, clientUser[pairedPartners[c]],
					pairedPartners[c], noOfBytesUsed[c]);

		}
	}

	printf("\nClient Blocked: %d\n", num_blocked);
	fprintf(fp, "\nClient Blocked: %d\n", num_blocked);
	printf(
			"______________________________________________________________________\n");
	fprintf(fp,
			"______________________________________________________________________\n");
	printf("Client username\t|\tSocket Number\n");
	fprintf(fp, "Client username\t|\tSocket Number\n");
	printf(
			"_______________________________________________________________________\n");
	fprintf(fp,
			"_______________________________________________________________________\n");

	for (c = 0; c < ACTIVE_LIST; c++) {
		if (blockedUsers[c] > 0) {
			printf(" %s \t | %d \n", clientUser[c], c);
			fprintf(fp, " %s \t | %d \n", clientUser[c], c);
		}
	}

	fclose(fp);
}

void block_client(int fdSocket) {
	char client1[100];
	char client2[100];
	strcpy(client1, "You are blocked by the server");
	strcpy(client2, "Your partner is blocked by the server");

	if ((pairedPartners[fdSocket] != fdSocket) && (fdSocket > -1)) {
		struct packet blockPacket;
		struct packet blockMsgPacket;

		strcpy(blockPacket.command, "SERVER_THROWOUT");
		strcpy(blockMsgPacket.command, "SERVER_THROWOUT");

		strcpy(blockPacket.message, client1);
		strcpy(blockMsgPacket.message, client2);

		sendDataPacket(fdSocket, &blockPacket);
		sendDataPacket(pairedPartners[fdSocket], &blockMsgPacket);
	} else if (pairedPartners[fdSocket] == fdSocket) {
		struct packet throw_packet;
		strcpy(throw_packet.command, "SERVER_THROWOUT");

		strcpy(throw_packet.message, client1);
		sendDataPacket(fdSocket, &throw_packet);
		removeChat(fdSocket, 1);
	}
	removeChat(pairedPartners[fdSocket], 0);
	removeChat(fdSocket, 0);
	removeUser(pairedPartners[fdSocket]);
	removeUser(fdSocket);

	blockedUsers[fdSocket] = 1;
}

void unblock_client(int fdSocket) {
	blockedUsers[fdSocket] = 0;

	struct packet client;

	strcpy(client.command, "ADMIN_RESPONSE");
	strcpy(client.username, "Server");
	strcpy(client.message,
			"You have been unblocked by the server. You can participate in chats again\n");
	sendDataPacket(fdSocket, &client);

}

void *handle_userIO(void *param, int socket_fd) {

	char command[COM_SIZE];

	while (fgets(command, sizeof command, stdin)) {

		int len = strlen(command);
		if (len > 0 && command[len - 1] == '\n') {
			command[len - 1] = '\0';
		}

		int index = 1;
		char* pch;

		char sub_command[2][COM_SIZE];
		memset(sub_command[0], "", COM_SIZE);
		memset(sub_command[1], "", COM_SIZE);

		int isEmpty = strlen(command);
		if (isEmpty == 0)
			continue;

		pch = strtok(command, " ");
		strcpy(sub_command[0], pch);
		while (pch != NULL) {
			pch = strtok(NULL, " ");
			if (pch != NULL) {
				strcpy(sub_command[1], pch);
				index++;
				if (index > 1)
					break;
			}
		}

		if (index == 1) {
			char* c1 = &sub_command[0];
			if (!strcmp(c1, "EXIT")) {
				printf("Exiting server. Good bye.\n");
				close(socket_fd);
				exit(0);
			} else if (!strcmp(c1, "HELP")) {
				printf(
						"\nEXIT - Exit the server\nSTART - Start the server\nSTATS - Get status of server. This will write a txt file too in the running folder.\nEND - End all chats and alert clients\nBLOCK - Block a specific client with a socket (Usage: BLOCK <socket number>. To know socket number, run STATS\nUNBLOCK - Unblock a specific client with a socket (Usage: UNBLOCK <socket number>. To know socket number, run STATS\nTHROWOUT - Throw a specific client with a socket (Usage: THORWOUT <socket number>. To know socket number, run STATS\n");
			} else if (!strcmp(c1, "START")) {
				if (startServer == 0) {
					startServer = 1;
					printf(
							"************SERVER has successfully start and now waiting for clients************\n");
				} else {
					printf("SERVER is alrady running\n");
				}
			} else if (!strcmp(c1, "STATS")) {
				if (startServer == 0)
					printf(
							"SERVER has not yet been STARTED. Please START first.\n");
				else
					print_server_stats();
			} else if (!strcmp(c1, "END")) {
				if (startServer == 1) {
					printf("SERVER is being killed......\n");
					end();
					printf("SERVER has successfully been killed\n");
				} else {
					printf(
							"SERVER has not yet been STARTED. Please START first.\n");
				}

			} else {
				printf(
						"Invalid command. Please type HELP for available command\n");
			}
		} else if (index == 2) {
			char* c1 = &sub_command[0];
			char* c2 = &sub_command[1];
			if (!strcmp(c1, "BLOCK")) {
				int socket = atoi(c2);
				printf("Blocking socket %d\n", socket);
				block_client(socket);
				printf("Blocked socket %d\n", socket);
			} else if (!strcmp(c1, "UNBLOCK")) {
				int socket = atoi(c2);
				printf("Unblocking socket %d\n", socket);
				unblock_client(socket);
				printf("Unblocked socket %d\n", socket);
			} else if (!strcmp(c1, "THROWOUT")) {
				int socket = atoi(c2);
				printf("Throwing out socket %d\n", socket);
				throwout(socket, 2);
				printf("Thrown out socket %d\n", socket);
			} else {
				printf(
						"Invalid Command. Please type HELP for available commands\n");
			}
		} else {
			printf(
					"Invalid Command. Please type HELP for available commands\n");
		}
	}

	return NULL;
}

int main() {

	startServer = 0;

	int incomingSocketNo, fdLast;
	struct addrinfo serverDetails, *serverInfo, *addressInfoPointer;
	char hostname[256];
	struct sockaddr_storage remoteAddress;
	socklen_t remoteAddressSize;
	char IP_remote[INET_ADDRSTRLEN];

	fd_set socketSet;
	fd_set temp;

	FD_ZERO(&temp);
	FD_ZERO(&socketSet);

	clearChatList();

	gethostname(hostname, 256);

	printf("Server hosted at host with name: %s\n", hostname);

	pthread_t thread_start;

	pthread_create(&thread_start, NULL, handle_userIO, NULL);

	printf("Server initiating.Type START to start the server. Press HELP for further commands.\n");

	while (!startServer)
		;

	int socketListener = serverHost(&serverDetails, &serverInfo,&addressInfoPointer, PORT_NO);

	FD_SET(socketListener, &socketSet);

	fdLast = socketListener;

	printf("\n----------------Server Started. Running on port %s------------------- *****\n",
			PORT_NO);

	int i;
	while (1) {

		temp = socketSet;
		if (select(fdLast + 1, &temp, NULL, NULL, NULL) == -1) {
			exit(4);
		}

		for (i = 0; i <= fdLast; i++) {
			if (FD_ISSET(i, &temp)) {
				if (i == socketListener) {
					remoteAddressSize = sizeof remoteAddress;
					incomingSocketNo = accept(socketListener,
							(struct sockaddr *) &remoteAddress,
							&remoteAddressSize);

					if (incomingSocketNo == -1)
						perror("Error on accepting connection");
					else {
						FD_SET(incomingSocketNo, &socketSet);
						if (incomingSocketNo > fdLast)
							fdLast = incomingSocketNo;

						char ipaddr[20];
						strcpy(ipaddr,
								inet_ntop(remoteAddress.ss_family,
										&(((struct sockaddr_in*) &remoteAddress)->sin_addr),
										IP_remote, INET_ADDRSTRLEN));
						printf("Incoming connection accepted from -- %s -- running on socket -- %d\n",
								ipaddr, incomingSocketNo);
						acceptClient(incomingSocketNo);
					}
				} else {
					unmarshellRequest(i, &socketSet);
				}
			}
		}
	}
	return 0;
}

