#include "Chatclient.h"

#define PORT_NO "5250"
#define MAX_DATA 1000
#define MAX_HOSTNAME_SIZE 100

char global_hostname[MAX_HOSTNAME_SIZE];

int main() {
	printf(
			"\n\n--------------------------------------------------------------------\n");
	printf(
			"------TYPE CONNECT TO START THE CHAT ROULLTTE CLIENT-----------------------\n");
	printf(
			"---------------------------------------------------------------------------------------------\n\n");

	struct addrinfo hints, *serverInfo, *p;
	int chatting = 0;
	paired = 0;

	char clientRequest[COM_SIZE];

	while (fgets(clientRequest, sizeof clientRequest, stdin)) {
		int lenCommand = strlen(clientRequest);
		if (lenCommand > 0 && clientRequest[lenCommand - 1] == '\n') {
			clientRequest[lenCommand - 1] = '\0';
		}

		if (strcmp(clientRequest, "CONNECT") == 0) {
			if (connection_active > 0) {
				printf(
						"An active connection is already running on %s at port %s\n",
						global_hostname, PORT_NO);
			} else {
				printf(
						"\nEnter hostname of server you are trying to connect to");
				scanf("%s", global_hostname);
				printf("Client connecting to %s......\n", global_hostname);
				connection_active = serverConnect(PORT_NO, &hints, &serverInfo,
						global_hostname, &p);

				if (connection_active > 0) {
					printf(
							"Connection to Chat server is established. The server is running on %s at port %s\n\n",
							global_hostname, PORT_NO);
				} else {
					fprintf(stderr,
							"Failed to reach the given server %s. Please check the hostname you are trying to connect.",
							global_hostname);
				}
			}
		}

		else if (strcmp(clientRequest, "CHAT") == 0) {
			if (connection_active > 0) {
				struct packet send_packet;

				if (createPacketForSending(clientRequest, &send_packet) == -1) {
					fprintf(stderr, "Try again.\n\n");
				} else {
					sendPacketData(&send_packet);
					chatting = 1;
				}

				memset(&send_packet, 0, sizeof(struct packet));
			} else {
				printf(
						"You are not connected to the TCR server. CONNECT first.\n\n");
			}
		} else if (strcmp(clientRequest, "TEXT") == 0) {
			if (connection_active > 0 && chatting) {
				struct packet send_packet;

				int return_genpacket = createPacketForSending(clientRequest,
						&send_packet);
				if (return_genpacket == -1) {
					fprintf(stderr,
							"Error occurred during packet creation. Try again.\n");
				} else if (return_genpacket == -2) {
					printf("You are not paired with anyone yet\n");
				} else {
					sendPacketData(&send_packet);
				}

				memset(&send_packet, 0, sizeof(struct packet));
			} else if (connection_active > 0 && !chatting) {
				printf("You need to enter the CHAT command first .\n\n");
			} else {
				printf(
						"You are not connected to the TCR server. CONNECT first.\n\n");
			}
		}

		else if (strcmp(clientRequest, "QUIT") == 0) {
			if (connection_active > 0) {
				struct packet send_packet;

				if (createPacketForSending(clientRequest, &send_packet) == -1) {
					fprintf(stderr, "Error occurred during packet creation.\n");
				} else {
					sendPacketData(&send_packet);
					chatting = 0;
					paired = 0;
				}

				memset(&send_packet, 0, sizeof(struct packet));
			} else {
				printf(
						"You are not connected to the TCR server. CONNECT first.\n\n");
			}
		} else if (strcmp(clientRequest, "EXIT") == 0) {
			printf("Exiting chat\n");
			break;
		} else if (strcmp(clientRequest, "HELP") == 0) {
			if (connection_active > 0) {
				struct packet send_packet;
				if (createPacketForSending(clientRequest, &send_packet) == -1) {
					fprintf(stderr, "Error occurred during packet creation.\n");
				} else {
					sendPacketData(&send_packet);
				}

				memset(&send_packet, 0, sizeof(struct packet));
			} else {
				printf("Connection to server is not established.\n\n");
			}
		} else {
			printf("Enter HELP to get the list of valid commands.\n\n");
		}
	}

	close(fd_socket);
	printf("Client is closed.\n");
	return 0;

}
