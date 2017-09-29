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
#define COM_SIZE 30
#define NAME_SIZE 30

int fd_socket;
int paired;
int connection_active = 0;

char username[NAME_SIZE];

struct packet {
    char username[NAME_SIZE];
    char message[MSG_SIZE];
    char clientCommand[COM_SIZE];
};

struct thread_data {
    pthread_t ptr_thread;
};

int createPacketForSending(const char *command, struct packet *send_packet);
int sendPacketData(struct packet *packet);
int serverConnect(char *port, struct addrinfo *hints, struct addrinfo **serverInfo,char *hostname, struct addrinfo **p);

void *receiveMessage(void *argument);
void handleReceivedMsg(struct packet *receivedMsg);

int serverConnect(char *port, struct addrinfo *hints, struct addrinfo **serverInfo, char *hostname, struct addrinfo **p) {

    memset(hints, 0, sizeof *hints);
    (*hints).ai_family = AF_INET;
    (*hints).ai_socktype = SOCK_STREAM;

    int status;
    getaddrinfo(hostname, port, hints, &(*serverInfo));

    for((*p) = (*serverInfo); (*p) != NULL; (*p) = (*p)->ai_next)
    {
        fd_socket = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol);
        if (connect(fd_socket, (*p)->ai_addr, (*p)->ai_addrlen) == -1)
        {
            close(fd_socket);
            continue;
        }

        break;
    }
    freeaddrinfo(*serverInfo);

    struct thread_data created_thread;
    pthread_create(&created_thread.ptr_thread, NULL, receiveMessage, (void *)&created_thread);
    fprintf(stdout, "Waiting for message at socket number [%i]\n", fd_socket);
    connection_active = 1;

    return 1;
}

void *receiveMessage(void *argument) {

    int isReceived;
    struct packet receivedMsg;
    connection_active = 1;

    struct packet send_packet;
    createPacketForSending("CONFIRM", &send_packet);
    sendPacketData(&send_packet);
    memset(&send_packet, 0, sizeof(struct packet));

    while(connection_active) {
        isReceived = recv(fd_socket, (void *)&receivedMsg, sizeof(struct packet), 0);

        if (!isReceived) {
            fprintf(stderr, "SERVER has hung up. Lost connection with server.\n");
            connection_active = 0;
            close(fd_socket);
            break;
        }

        if (isReceived > 0) {
            fflush(stdout);
            handleReceivedMsg(&receivedMsg);
        }
        memset(&receivedMsg, 0, sizeof(struct packet));
    }
}

void handleReceivedMsg(struct packet *receivedMsg) {

	if (strcmp((*receivedMsg).clientCommand, "ACKN") == 0
			|| strcmp((*receivedMsg).clientCommand, "QUIT") == 0
			|| strcmp((*receivedMsg).clientCommand, "HELP") == 0
			|| strcmp((*receivedMsg).clientCommand, "SERVER_MSG") == 0) {
		printf("%s\n", (*receivedMsg).message);
	} else if (strcmp((*receivedMsg).clientCommand, "IN SESSION") == 0) {
		printf("%s\n", (*receivedMsg).message);
		paired = 1;
	}

	else if (strcmp((*receivedMsg).clientCommand, "TEXT") == 0) {
		printf("%s :::>>>> %s\n", (*receivedMsg).username,
				(*receivedMsg).message);
	} else if (strcmp((*receivedMsg).clientCommand, "SERVER_THROWOUT") == 0) {
		printf("%s\n", (*receivedMsg).message);
		paired = 0;
		if (paired == 0) {
			printf("You are not paired with anyone");
		}
	}
}

int createPacketForSending(const char *command, struct packet *send_packet) {

	struct packet outgoing_packet;
	strncpy(outgoing_packet.clientCommand, command, COM_SIZE);
	strncpy(outgoing_packet.username, "username", NAME_SIZE);
	strncpy(outgoing_packet.message, "message", MSG_SIZE);

	if (strcmp(command, "CHAT") == 0) {

		char name_input[NAME_SIZE];
		printf("Enter name: ");
		fgets(name_input, sizeof name_input, stdin);

		int lenUsername = strlen(name_input);

		if (name_input == NULL || strcmp(name_input, "\n") == 0
				|| strcmp(name_input, " ") == 0) {
			fprintf(stderr,
					"Name cannot be blank or contain special characters.\n");
			return -1;
		}

		if (lenUsername > 0 && name_input[lenUsername - 1] == '\n') {
			name_input[lenUsername - 1] = '\0';
		}

		strncpy(outgoing_packet.username, name_input, NAME_SIZE);
		strncpy(username, name_input, NAME_SIZE);

		(*send_packet) = outgoing_packet;
		return 1;
	}

	else if (strcmp(command, "TEXT") == 0) {
		if (paired == 0) {
			return -2;
		} else {
			fprintf(stdout, "YOU>>>>>>>> ");
			fgets(outgoing_packet.message, sizeof outgoing_packet.message,
					stdin);

			int lenOutgoingPacket = strlen(outgoing_packet.message);
			if (lenOutgoingPacket > 0
					&& outgoing_packet.message[lenOutgoingPacket - 1] == '\n') {
				outgoing_packet.message[lenOutgoingPacket - 1] = '\0';
			}

			strncpy(outgoing_packet.username, username, NAME_SIZE);
			(*send_packet) = outgoing_packet;
			return 1;
		}
	} else if (strcmp(command, "QUIT") == 0) {
		strncpy(outgoing_packet.username, username, NAME_SIZE);
		(*send_packet) = outgoing_packet;
		return 1;
	} else if (strcmp(command, "CONNECT") == 0 || strcmp(command, "HELP") == 0
			|| strcmp(command, "CONFIRM") == 0) {
		(*send_packet) = outgoing_packet;
		return 1;
	}
}

int sendPacketData(struct packet *sendPacket) {
	int sendPacketLen = sizeof *sendPacket;
	int total = 0;
	int countDataBytes = sendPacketLen;
	int sentByteCounter, success;

	while (total < sendPacketLen) {
		sentByteCounter = send(fd_socket, (sendPacket + total), sentByteCounter,
				0);
		;
		if (sentByteCounter == -1) {
			break;
		}
		total = total + sentByteCounter;
		countDataBytes = countDataBytes - sentByteCounter;
	}
	memset(sendPacket, 0, sizeof(struct packet));
	if (sentByteCounter == -1)
		success = -1;
	else
		success = 0;
	return success;

}
