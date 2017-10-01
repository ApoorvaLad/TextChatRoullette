#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>

#define MSG_SIZE 1024
#define COM_SIZE 30
#define ACTIVE_LIST 10

#define BACKLOG 20
#define PORT_NO "5230"

#define MAX_BUFFER_LEN 2048
#define NAME_LEN 20

struct packet {
	char username[NAME_LEN];
	char message[MSG_SIZE];
	char command[COM_SIZE];
};

int startServer;
int pairedPartners[ACTIVE_LIST];
int blockedUsers[ACTIVE_LIST];
int noOfBytesUsed[ACTIVE_LIST];
char clientUser[ACTIVE_LIST][NAME_LEN];

void removeChat(int fdSocket, int reset);
void removeChats(int senderSocket, int receiverSocket);
int serverHost(struct addrinfo *serverDetails, struct addrinfo **serverInfo,
		struct addrinfo **addressInfoPointer, char *portNo);
int serverHost(struct addrinfo *serverDetails, struct addrinfo **serverInfo,
		struct addrinfo **addressInfoPointer, char *portNo) {
	int error_status, listener;
	int opt_value = 1;

	memset(serverDetails, 0, sizeof *serverDetails);
	(*serverDetails).ai_family = AF_INET;
	(*serverDetails).ai_socktype = SOCK_STREAM;

	if ((error_status = getaddrinfo(NULL, portNo, serverDetails, &(*serverInfo)))
			!= 0) {
		printf("Failed to fetch address information");
		exit(1);
	}

	addressInfoPointer = serverInfo;
	while ((*addressInfoPointer) != NULL) {
		if ((listener = socket((*addressInfoPointer)->ai_family,
				(*addressInfoPointer)->ai_socktype,
				(*addressInfoPointer)->ai_protocol)) == -1) {
			(*addressInfoPointer) = (*addressInfoPointer)->ai_next;
			continue;
		}
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt_value,
				sizeof(int)) == -1) {
			exit(1);
		}
		if (bind(listener, (*addressInfoPointer)->ai_addr,
				(*addressInfoPointer)->ai_addrlen) == -1) {
			close(listener);
			(*addressInfoPointer) = (*addressInfoPointer)->ai_next;
			continue;
		}
		break;
	}

	freeaddrinfo(*serverInfo);

	if ((*addressInfoPointer) == NULL) {
		printf("Binding failed. Please check server for multiple instances.\n");
		exit(2);
	}
	if (listen(listener, BACKLOG) == -1) {
		printf("Listening failed. Please check server configuration.\n");
		exit(3);
	}
	return listener;
}

int removeUser(int fdSocket) {
	if (fdSocket >= ACTIVE_LIST) {
		perror("Socket not correct");
		return -1;
	}
	return 1;
}

void removeAllUsers() {
	int c = 0;
	while (c < ACTIVE_LIST) {
		removeUser(c);
		c++;
	}
}

void clearChatList() {

	int c = 0;
	while (c < ACTIVE_LIST) {
		pairedPartners[c] = -2;
		c++;
	}

}

void removeSocket(int fdSocket) {
	printf("Removing socket %d\n", fdSocket);
	if (removeUser(fdSocket) == 1) {
		removeChat(pairedPartners[fdSocket], 1);
		pairedPartners[fdSocket] = -2;
		blockedUsers[fdSocket] = 0;
	}
}

int socketClose(int fdSocket, int flag, int free_socket, fd_set *master) {
	removeSocket(fdSocket);
	if (free_socket) {
		close(fdSocket);
		FD_CLR(fdSocket, master);
		return 1;
	}
	int success = shutdown(fdSocket, flag);
	if (success == 0) {
		FD_CLR(fdSocket, master);
		return 1;
	}
	return 0;
}

void acceptClient(int fdSocket) {
	if (fdSocket >= ACTIVE_LIST) {
		perror("Chat list full. Please try after sometime\n");
	} else {
		printf("Socket number -- %d -- added to chat queue\n", fdSocket);
		pairedPartners[fdSocket] = -1;
	}
}

void launchNewConnection(int fdSocket) {
	struct packet ack_packet;
	strcpy(ack_packet.command, "ACKN");

	if (fdSocket >= ACTIVE_LIST) {
		perror("Chat list full. Please try after sometime\n");
		strcpy(ack_packet.message,
				"Chat list full. Please try after sometime\n");
	} else {
		if (pairedPartners[fdSocket] == fdSocket
				|| pairedPartners[fdSocket] == -1) {
			printf("User with socket number %d added to chat queue\n",
					fdSocket);
			strcpy(ack_packet.message, "You are on the chat queue");
		} else if (pairedPartners[fdSocket] != -2) {
			printf("Socket number -- %d -- added to chat queue\n", fdSocket);
			strcpy(ack_packet.message, "You are on a chat queue");
		}
	}
	if (sendDataPacket(fdSocket, &ack_packet) == -1)
		perror("send");
}

void asignUserAlias(int fdSocket1, int fdSocket2, char* username) {
	if (fdSocket1 >= ACTIVE_LIST || fdSocket2 >= ACTIVE_LIST) {
		perror("Socket is invalid");
		return;
	}

	char* user2 = &clientUser[fdSocket2][0];

	int compareTo = strcmp(username, user2);
	if (compareTo == 0) {
		username = strcat(username, "(duplicate)");
		printf("Username already exists. Assign a new username: %s\n",
				username);
	}
}
void sendDataPacket(int receiverSocket, struct packet *sendPacket) {

	int packetLength = sizeof *sendPacket;
	int totalBytesSent = 0;
	int totalBytesLeft = packetLength;
	int n;

	while (totalBytesSent < packetLength) {

		n = send(receiverSocket, (sendPacket + totalBytesSent), totalBytesLeft,
				0);

		if (n == -1) {
			break;
		}

		totalBytesSent += n;
		totalBytesLeft -= n;

	}

	memset(sendPacket, 0, sizeof(struct packet));
	if (n == -1) {
		printf("Error while sending data \n\n");
		return;
	}
}

void initiateChat(int senderSocket, int receiverSocket, char* username) {
	if (senderSocket >= ACTIVE_LIST || receiverSocket >= ACTIVE_LIST) {
		perror("Given socket is unavailable");
		return;
	}

	asignUserAlias(senderSocket, receiverSocket, username);
	strcpy(clientUser[senderSocket], username);
	pairedPartners[receiverSocket] = senderSocket;
	pairedPartners[senderSocket] = receiverSocket;

	struct packet receivePacket, sendPacket;
	strcpy(receivePacket.command, "KEEP ALIVE");
	strcpy(sendPacket.command, "KEEP ALIVE");
	strcpy(receivePacket.message,
			"You are chatting with -------------------------------- ");
	strcat(receivePacket.message, clientUser[senderSocket]);
	strcat(receivePacket.message, "\n Type CHAT and start chatting");
	strcpy(sendPacket.message,
			"You are chatting with -------------------------------- ");
	strcat(sendPacket.message, clientUser[receiverSocket]);
	strcat(sendPacket.message, "\n Type CHAT and start chatting");
	sendDataPacket(receiverSocket, &receivePacket);
	sendDataPacket(senderSocket, &sendPacket);
}

int activateChat(int senderSocket, char* username) {
	if (senderSocket >= ACTIVE_LIST) {
		perror("Chat list full. Please try after sometime\n");
		return -1;
	}

	int i;
	int connections = 0;
	int availableSocket = -1;
	for (i = 0; i < ACTIVE_LIST; i++) {
		if (pairedPartners[i] == i)
			connections++;
	}

	if (connections > 0) {
		int j;
		int availableSocket = -1;
		for (j = 0; j < ACTIVE_LIST; j++) {
			if (pairedPartners[j] == j && j != senderSocket) {
				availableSocket = j;
				break;
			}
		}

		initiateChat(senderSocket, availableSocket, username);
	} else {
		pairedPartners[senderSocket] = senderSocket;
		strcpy(clientUser[senderSocket], username);
		struct packet receiverPacket, senderPacket;
		strcpy(senderPacket.command, "ADMIN_RESPONSE");
		strcpy(senderPacket.message, "Waiting for another user to connect");
		sendDataPacket(senderSocket, &senderPacket);

	}
	return 1;
}

void removeChat(int fdSocket, int reset) {
	if (fdSocket > 0 && pairedPartners[fdSocket] > -1) {
		printf("Terminating chat channel of socket %d\n", fdSocket);
		struct packet dataPacket;
		strcpy(dataPacket.command, "QUIT");
		if (pairedPartners[fdSocket] != fdSocket) {
			strcpy(dataPacket.message, "Removed from the chat with partner: ");
			strcat(dataPacket.message, clientUser[pairedPartners[fdSocket]]);
		} else {
			printf("User with socket -- %d -- is thrown out of the chat \n",
					fdSocket);
			strcpy(dataPacket.message, "You are thrown out of the chat\n");
		}
		strcat(dataPacket.message, "\nYou are back in the chat queue\n");
		sendDataPacket(fdSocket, &dataPacket);

		pairedPartners[fdSocket] = -1;
		noOfBytesUsed[fdSocket] = 0;

		if (reset == 1)
			removeUser(fdSocket);
	}
}

void removeChats(int senderSocket, int receiverSocket) {
	removeChat(receiverSocket, 0);
	removeChat(senderSocket, 0);
	removeUser(receiverSocket);
	removeUser(senderSocket);
}

void messageHandler(int senderSocket, struct packet *msgReceived) {
	struct packet dataPacket;

	if (pairedPartners[senderSocket] < 0
			|| pairedPartners[senderSocket] == senderSocket) {
		strcpy(dataPacket.command, "ADMIN_RESPONSE");
		strcpy(dataPacket.username, "Server");
		strcpy(dataPacket.message,
				"You are not connected. Please CONNECT first.");
		sendDataPacket(senderSocket, &dataPacket);
	} else {
		dataPacket = *msgReceived;
		noOfBytesUsed[senderSocket] += sizeof(*msgReceived);
		noOfBytesUsed[pairedPartners[senderSocket]] += sizeof(*msgReceived);
		sendDataPacket(pairedPartners[senderSocket], &dataPacket);
	}
}

void handleTransferData(int senderSocket, struct packet *msgReceived) {
	struct packet dataPacket;

	if (pairedPartners[senderSocket] < 0
			|| pairedPartners[senderSocket] == senderSocket) {
		strcpy(dataPacket.command, "ADMIN_RESPONSE");
		strcpy(dataPacket.username, "Server");
		strcpy(dataPacket.message,
				"You are not connected. Please CONNECT first.");
		sendDataPacket(senderSocket, &dataPacket);
	} else {
		dataPacket = *msgReceived;
		noOfBytesUsed[senderSocket] += sizeof(*msgReceived);
		noOfBytesUsed[pairedPartners[senderSocket]] += sizeof(*msgReceived);
		sendDataPacket(pairedPartners[senderSocket], &dataPacket);
	}
}

void processIncomingRequest(int fdSocket, struct packet *msgReceived) {
	printf(msgReceived->message);
	if (!strcmp(msgReceived->command, "BEGIN")) {
		if (blockedUsers[fdSocket] > 0) {
			struct packet send_packet;

			strcpy(send_packet.command, "ADMIN_RESPONSE");
			strcpy(send_packet.username, "Server");
			strcpy(send_packet.message,
					"Your current status is BLOCKED. Wait for UNBLOCK status");
			sendDataPacket(fdSocket, &send_packet);
		} else {
			int possible_socket = activateChat(fdSocket, msgReceived->username);
			if (possible_socket < 1) {
				struct packet send_packet;

				strcpy(send_packet.command, "ADMIN_RESPONSE");
				strcpy(send_packet.username, "Server");
				strcpy(send_packet.message, "Chat could not be initiated");
				sendDataPacket(fdSocket, &send_packet);
			}
		}
	} else if (!strcmp(msgReceived->command, "LAUNCH")) {
		launchNewConnection(fdSocket);
	} else if (!strcmp(msgReceived->command, "QUIT")) {
		removeChats(fdSocket, pairedPartners[fdSocket]);
	} else if (!strcmp(msgReceived->command, "CHAT")) {
		messageHandler(fdSocket, msgReceived);
	} else if (!strcmp(msgReceived->command, "TRANSFER")) {
		handleTransferData(fdSocket, msgReceived);
	} else if (!strcmp(msgReceived->command, "HELP")) {
		struct packet help_packet;
		strcpy(help_packet.command, "HELP");
		strcpy(help_packet.username, "Server");
		strcpy(help_packet.message,
				"\n- CONNECT: Connect to server\n- CHAT: Start chatting\n- BEGIN: Request chat partner\n- QUIT: End chatting\n- EXIT: Exit TCR chat client\n- HELP: Usage");
		sendDataPacket(fdSocket, &help_packet);

	}
}

void unmarshellRequest(int fdSocket, fd_set *master) {

	int num_bytes;
	char buf[MAX_BUFFER_LEN];

	struct packet received_message;
	if ((num_bytes = recv(fdSocket, (void *) &received_message,
			sizeof(struct packet), 0)) <= 0) {
		socketClose(fdSocket, SO_KEEPALIVE, 1, master);
	} else {
		processIncomingRequest(fdSocket, &received_message);
	}
}
