#include "server.h"


void blockUser(int fdSocket) {
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

void unblockUser(int fdSocket) {
	blockedUsers[fdSocket] = 0;
	struct packet client;
	strcpy(client.command, "ADMIN_RESPONSE");
	strcpy(client.username, "Server");
	strcpy(client.message,"Your status has been unblocked by the server.S\n");
	sendDataPacket(fdSocket, &client);

}

void throwout(int fdSocket) {

	char person1[100];
	char person2[100];
	strcpy(person1, "Server has issued - THROWOUT");
	strcpy(person2, "Sever has issued - THROWOUT on your partner");

	strcpy(person1, "Server has stopped your active chat");

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

void endServer() {
	int c;
	for (c = 0; c < ACTIVE_LIST; c++) {
		if (pairedPartners[c] > -2) {
			throwout(c);
		}
		pairedPartners[c] = -2;
		blockedUsers[c] = 0;
		noOfBytesUsed[c] = 0;
		memset(&clientUser[c], 0, COM_SIZE * sizeof(clientUser[0]));
	}
	startServer = 0;
}



void showStats() {
	int itr;
	int chatQueueCount = 0;
	int chattingUserCount = 0;
	int blockedUserCount = 0;
	FILE *file_ptr;

	file_ptr = fopen("serverCurrserverStats.txt", "w");
	for (itr = 0; itr < ACTIVE_LIST; itr++) {
		if (pairedPartners[itr] == -1 || pairedPartners[itr] == itr)
			chatQueueCount++;
		else if (pairedPartners[itr] != itr && pairedPartners[itr] != -2) {
			chattingUserCount++;
		}
		if (blockedUsers[itr] > 0)
			blockedUserCount++;
	}


	printf("------------------SERVER STATS----------------------\n");

	fprintf(file_ptr, "--------------------SERVER STATS-------------------------\n");

	printf("\nUsers on queue: %d\n", chatQueueCount);
	fprintf(file_ptr, "\nUsers on queue: %d\n", chatQueueCount);
	printf("____________________________________________\n");
	fprintf(file_ptr, "____________________________________________\n");

	printf("User\t:\tSocket Number\n");
	fprintf(file_ptr, "User\t:\tSocket Number\n");
	printf("____________________________________________\n");
	fprintf(file_ptr, "____________________________________________\n");

	printf("----------CHAT QUEUE SUMMARY-------------------------------\n");
	fprintf(file_ptr, "-------------CHAT QUEUE SUMMARY-------------------------------\n");

	for (itr = 0; itr < ACTIVE_LIST; itr++) {
		if (pairedPartners[itr] == -1) {
			printf("%s\t\t:\t\t%d\n", "No Users Active", itr);
			fprintf(file_ptr, "%s\t\t:\t\t%d\n", "No Users Active", itr);
		} else if (pairedPartners[itr] == itr) {
			printf(" %s \t : %d \n", clientUser[itr], itr);
			fprintf(file_ptr, " %s \t : %d \n", clientUser[itr], itr);
		}
	}

		printf("----------ACTIVE USERS-------------------------------\n");
		fprintf(file_ptr, "-------------ACTIVE USERS-------------------------------\n");

		printf("\nNo of Active Users Chatting: %d\n", chattingUserCount);
		fprintf(file_ptr, "\nNo of Active Users Chatting: %d\n", chattingUserCount);
		printf("______________________________________________________________________\n");
		fprintf(file_ptr,"______________________________________________________________________\n");
		printf("User One : Socket One -> User Two : Socket Two :::: Bytes Used\n");
		fprintf(file_ptr,"User One : Socket One -> User Two : Socket Two ::: Bytes Used\n");
		printf("_______________________________________________________________________\n");
		fprintf(file_ptr,"_______________________________________________________________________\n");

		for (itr = 0; itr < ACTIVE_LIST; itr++) {
			if (pairedPartners[itr] != -1 && pairedPartners[itr] != itr
					&& pairedPartners[itr] != -2) {

				printf("%s\t\t:\t\t %d\t\t -> \t\t%s\t\t:\t\t%d\t\t::::\t%d\n",
						clientUser[itr], itr, clientUser[pairedPartners[itr]],
						pairedPartners[itr], noOfBytesUsed[itr]);
				fprintf(file_ptr, "%s\t\t:\t\t %d\t\t -> \t\t%s\t\t:\t\t%d\t\t::::\t%d\n",
						clientUser[itr], itr, clientUser[pairedPartners[itr]],
						pairedPartners[itr], noOfBytesUsed[itr]);

			}
		}



	printf("----------BLOCKED USERS-------------------------------\n");
	fprintf(file_ptr, "-------------BLOCKED USERS-------------------------------\n");

	printf("\nUsers Blocked: %d\n", blockedUserCount);
	fprintf(file_ptr, "\nUsers Blocked: %d\n", blockedUserCount);
	printf("______________________________________________________________________\n");
	fprintf(file_ptr,"______________________________________________________________________\n");
	printf("User One : Socket One -> User Two : Socket Two\n");
	fprintf(file_ptr,"User One : Socket One -> User Two : Socket Two\n");
	printf("_______________________________________________________________________\n");
	fprintf(file_ptr,"_______________________________________________________________________\n");

	for (itr = 0; itr < ACTIVE_LIST; itr++) {
		if (blockedUsers[itr] > 0) {
			printf(" %s \t : %d \n", clientUser[itr], itr);
			fprintf(file_ptr, " %s \t : %d \n", clientUser[itr], itr);
		}
	}

	fclose(file_ptr);
}

void *receivedMessage(void *param, int fdSocket) {

	char command[COM_SIZE];

	while (fgets(command, sizeof command, stdin)) {

		int len = strlen(command);
		if (len > 0 && command[len - 1] == '\n') {
			command[len - 1] = '\0';
		}
		int isEmpty = strlen(command);
		if (isEmpty == 0)
			continue;

		int noOfArguements = 1;
		char* commandTokenizer;
		char commSplitArray[2][COM_SIZE];
		memset(commSplitArray[0], 0, COM_SIZE);
		memset(commSplitArray[1], 0, COM_SIZE);

		commandTokenizer = strtok(command, " ");
		strcpy(commSplitArray[0], commandTokenizer);
		while (commandTokenizer != NULL) {
			commandTokenizer = strtok(NULL, " ");
			if (commandTokenizer != NULL) {
				strcpy(commSplitArray[1], commandTokenizer);
				noOfArguements++;
				if (noOfArguements > 1)
					break;
			}
		}

		if (noOfArguements == 1) {
			if (strcmp(commSplitArray[0], "START") == 0) {
				if (startServer == 0) {
					startServer = 1;
					printf("------------Server Started. Waiting for Clients to Connect--------\n");
				} else {
					printf("Server instance already running. Please check console for more information\n");
				}
			} else if (strcmp(commSplitArray[0], "STATS") == 0) {
				if (startServer == 0)
					printf("SERVER has not yet been STARTED. Please START first.\n");
				else
					showStats();
			} else if (strcmp(commSplitArray[0], "END") == 0) {
				if (startServer == 1) {
					printf("Server is being stopped......\n");
					endServer();
					printf("Server been stopped\n");
				} else {
					printf("Server instance has not been started yet.\n");
				}
			} else if (strcmp(commSplitArray[0], "EXIT") == 0) {
				printf("Server has stopped.\n");
				close(fdSocket);
				exit(0);
			} else if (strcmp(commSplitArray[0], "HELP") == 0) {
				printf("\nSTART - Starts the server\nSTATS - Get the updates about Users and socket running on this Server.\nEND - End all active chats and alerts users about it.\nEXIT - Exits the server.\nBLOCK - Block a specific user running on a socket of server (TIP: BLOCK <socket number>. To know socket number, PRESS STATS).\nUNBLOCK - Unblock a previously BLOCKED user running on a socket on server (TIP: UNBLOCK <socket number>. To know socket number, PRESS STATS).\nTHROWOUT - 'Throw' an active user out of the chat list(TIP: THROWOUT <socket number>. To know socket number, run STATS)\n");
			} else {
				printf("Command unrecognized. Press HELP for more information\n");
			}
		} else if (noOfArguements == 2) {
			if (strcmp(commSplitArray[0], "BLOCK") == 0) {
				int socket = atoi(&commSplitArray[1]);
				blockUser(socket);
				printf("Blocked user socket %d\n", socket);
			} else if (strcmp(commSplitArray[0], "UNBLOCK") == 0) {
				int socket = atoi(&commSplitArray[1]);
				unblockUser(socket);
				printf("Unblocked user on socket %d\n", socket);
			} else if (strcmp(commSplitArray[0], "THROWOUT") == 0) {
				int socket = atoi(&commSplitArray[1]);
				throwout(socket);
				printf("User thrown out of socket %d\n", socket);
			} else {
				printf("Command unrecognized. Press HELP for more information\n");
			}
		} else {
			printf("Command unrecognized. Press HELP for more information\n");
		}
	}

	return NULL;
}


int serverConnect() {
	struct addrinfo hints, *servinfo, *p;
	int listener, error_status, opt_value = 1;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	if ((error_status = getaddrinfo(NULL, PORT_NO, &hints, &servinfo)) != 0) {
		printf("Cannot get address info exiting server now");
		exit(1);
	}

	p = servinfo;
	while (p != NULL) {
		if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			p = p->ai_next;
			continue;
		}
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt_value,
				sizeof(int)) == -1) {
			exit(1);
		}
		if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
			close(listener);
			p = p->ai_next;
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);
	if (p == NULL) {
		printf(
				"FAILED to bind. EXITING server. This can happen when you have already run another instance of TCR server on the same machine. Please check it.\n");
		exit(2);
	}
	if (listen(listener, BACKLOG) == -1) {
		printf("FAILED to listen. EXITING server.\n");
		exit(3);
	}
	return listener;
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

	pthread_create(&thread_start, NULL, receivedMessage, NULL);

	printf("Server initiating.Type START to start the server. Press HELP for further commands.\n");

	while (!startServer)
		;

	int socketListener = serverConnect();

	FD_SET(socketListener, &socketSet);

	fdLast = socketListener;

	printf("\n----------------Server Started. Running on port %s-------------------------\n",
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
