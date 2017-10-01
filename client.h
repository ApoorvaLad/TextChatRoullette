#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MSG_SIZE 1024
#define COM_SIZE 20
#define NAME_SIZE 20

struct datapacket {
	char username[NAME_SIZE];
	char message[MSG_SIZE];
	char command[COM_SIZE];
};

struct thread_data {
	pthread_t ptr_thread;
};

int fdSocket;
int paired;
int activeConnection = 0;

char username[NAME_SIZE];
char fileLocation[MSG_SIZE];

int serverConnect(struct addrinfo *serverDetails,struct addrinfo **serverInfo,struct addrinfo **addressInfoPointer, char *hostname, char *portNo );
int createPacketForSending(const char *command, struct datapacket *dataPacket);
int sendPacketData(struct datapacket *packet);
void *getMessage(void *commandFlag);
void messageHandler(struct datapacket *dataReceived);

int serverConnect(struct addrinfo *serverDetails,struct addrinfo **serverInfo,struct addrinfo **addressInfoPointer, char *hostname, char *portNo ) {
	memset(serverDetails, 0, sizeof *serverDetails);
	(*serverDetails).ai_family = AF_INET;
	(*serverDetails).ai_socktype = SOCK_STREAM;
	int status;
	getaddrinfo(hostname, portNo, serverDetails, &(*serverInfo));

	for ((*addressInfoPointer) = (*serverInfo); (*addressInfoPointer) != NULL; (*addressInfoPointer) = (*addressInfoPointer)->ai_next) {
		fdSocket = socket((*addressInfoPointer)->ai_family, (*addressInfoPointer)->ai_socktype,
				(*addressInfoPointer)->ai_protocol);
		if (connect(fdSocket, (*addressInfoPointer)->ai_addr, (*addressInfoPointer)->ai_addrlen) == -1) {
			close(fdSocket);
			continue;
		}

		break;
	}
	freeaddrinfo(*serverInfo);

	struct thread_data created_thread;
	pthread_create(&created_thread.ptr_thread, NULL, getMessage,
			(void *) &created_thread);
	fprintf(stdout, "Waiting for message at socket number [%i]\n", fdSocket);
	activeConnection = 1;

	return 1;
}

int createPacketForSending(const char *command, struct datapacket *dataPacket) {

	struct datapacket packetToSend;
	strncpy(packetToSend.username, "username", NAME_SIZE);
	strncpy(packetToSend.message, "message", MSG_SIZE);
	strncpy(packetToSend.command, command, COM_SIZE);

	if (strcmp(command, "BEGIN") == 0) {

		char nameInput[NAME_SIZE];
		printf("Enter name: ");
		fgets(nameInput, sizeof nameInput, stdin);

		int lenUsername = strlen(nameInput);

		if (nameInput == NULL || strcmp(nameInput, "\n") == 0
				|| strcmp(nameInput, " ") == 0) {
			fprintf(stderr,"Name cannot be blank or contain special characters.\n");
			return -1;
		}

		if (lenUsername > 0 && nameInput[lenUsername - 1] == '\n') {
			nameInput[lenUsername - 1] = '\0';
		}

		strncpy(packetToSend.username, nameInput, NAME_SIZE);
		strncpy(username, nameInput, NAME_SIZE);

		(*dataPacket) = packetToSend;
		return 1;
	}

	else if (strcmp(command, "CHAT") == 0) {
		if (paired == 0) {
			return -2;
		} else {
			fprintf(stdout, "ME ::: ");
			fgets(packetToSend.message, sizeof packetToSend.message,stdin);
			int lenOutgoingPacket = strlen(packetToSend.message);
			if (lenOutgoingPacket > 0
					&& packetToSend.message[lenOutgoingPacket - 1] == '\n') {
				packetToSend.message[lenOutgoingPacket - 1] = '\0';
			}
			strncpy(packetToSend.username, username, NAME_SIZE);
			(*dataPacket) = packetToSend;
			return 1;
		}
	} else if (strcmp(command, "TRANSFER") == 0) {
		if (paired == 0) {
			return -2;
		} else {
			char file_location[MSG_SIZE];
			fprintf(stdout, "\nEnter the location of file to be Transfered");
			fgets(file_location, sizeof packetToSend.message, stdin);
			int lenOutgoingPacket = strlen(file_location);
			if (lenOutgoingPacket > 0
					&& file_location[lenOutgoingPacket - 1] == '\n') {
				file_location[lenOutgoingPacket - 1] = '\0';
			}
			strncpy(packetToSend.command, command, COM_SIZE);
			strncpy(packetToSend.message, file_location, MSG_SIZE);
			strncpy(packetToSend.username, username, NAME_SIZE);
			(*dataPacket) = packetToSend;
			return 1;
		}
	} else if (strcmp(command, "QUIT") == 0) {
		strncpy(packetToSend.username, username, NAME_SIZE);
		(*dataPacket) = packetToSend;
		return 1;
	} else if (strcmp(command, "CONNECT") == 0 || strcmp(command, "HELP") == 0
			|| strcmp(command, "LAUNCH") == 0) {
		(*dataPacket) = packetToSend;
		return 1;
	}

}

void sendDataPacket(struct datapacket *packetToSend) {
	int sizeOfPacket = sizeof *packetToSend;
	int totalBytesSent = 0;
	int totalBytesLeft = sizeOfPacket;
	int returnBytes;

	while (totalBytesSent < sizeOfPacket) {

		returnBytes = send(fdSocket, (packetToSend + totalBytesSent),
				totalBytesLeft, 0);
		if (returnBytes == -1) {
			break;
		}
		totalBytesSent = totalBytesSent + returnBytes;
		totalBytesLeft = totalBytesLeft - returnBytes;
	}
	memset(packetToSend, 0, sizeof(struct datapacket));
	if (returnBytes == -1)
		printf("Error while sending data \n\n");
	return;

}

void *getMessage(void *commandFlag) {

	struct datapacket receivedMsg;
	activeConnection = 1;
	int messageReceived;

	struct datapacket dataPacket;
	createPacketForSending("LAUNCH", &dataPacket);
	sendDataPacket(&dataPacket);

	memset(&dataPacket, 0, sizeof(struct datapacket));

	while (activeConnection) {
		messageReceived = recv(fdSocket, (void *) &receivedMsg,sizeof(struct datapacket), 0);
		if (!messageReceived) {
			fprintf(stderr,
					"SERVER has hung up. Lost connection with server.\n");
			activeConnection = 0;
			close(fdSocket);
			break;
		}

		if (messageReceived > 0) {
			fflush(stdout);
			messageHandler(&receivedMsg);
		}
		memset(&receivedMsg, 0, sizeof(struct datapacket));
	}
}

void messageHandler(struct datapacket *receivedMsg) {
	if (!strcmp((*receivedMsg).command, "ACKN")) {
			printf("%s\n", (*receivedMsg).message);
		} else if (!strcmp((*receivedMsg).command, "KEEP ALIVE")) {
			printf("%s\n", (*receivedMsg).message);
			paired = 1;
		} else if (!strcmp((*receivedMsg).command, "QUIT")) {
			printf("%s\n", (*receivedMsg).message);
		} else if (!strcmp((*receivedMsg).command, "HELP")) {
			printf("%s\n", (*receivedMsg).message);
		} else if (!strcmp((*receivedMsg).command, "CHAT")) {
			printf("%s ::: %s\n", (*receivedMsg).username,
					(*receivedMsg).message);
		} else if (!strcmp((*receivedMsg).command, "ADMIN_RESPONSE")) {
			printf("%s\n", (*receivedMsg).message);

		} else if (!strcmp((*receivedMsg).command, "SERVER_THROWOUT")) {
			printf("%s\n", (*receivedMsg).message);
			paired = 0;
			if (paired == 0) {
				printf("You are not paired with anyone");
			}
		}
}


void sendFileData(struct datapacket *sendPacket) {
	char msg[MSG_SIZE];

	strncpy(sendPacket->message, msg, MSG_SIZE);
	FILE *fp = fopen(msg, "rb");
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);  //same as rewind(f);

	char *string = malloc(fsize + 1);
	fread(string, fsize, 1, fp);
	fclose(fp);
	string[fsize] = 0;
	strncpy(sendPacket->message, string, MSG_SIZE);
	sendDataPacket(&sendPacket);
}


