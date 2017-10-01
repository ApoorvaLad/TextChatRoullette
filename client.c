#include "client.h"

#define PORT_NO "5230"
#define HOSTNAME_SIZE 50

char hostName[HOSTNAME_SIZE];

int main() {
	printf("\n\n--------------------------------------------------------------------\n");
	printf("------TYPE CONNECT TO START THE BEGIN ROULLTTE CLIENT-----------------------\n");
	printf("---------------------------------------------------------------------------------------------\n\n");

	struct addrinfo serverDetails,*serverInfo,*addressInfoPointer;
	int chatting = 0;
	paired = 0;

	char command[COM_SIZE];

	while (fgets(command, sizeof command, stdin)) {
		int lenCommand = strlen(command);
		if (lenCommand > 0 && command[lenCommand - 1] == '\n') {
			command[lenCommand - 1] = '\0';
		}

		if (strcmp(command, "CONNECT") == 0) {
			if (activeConnection > 0) {
				printf("An active connection is already running on %s at port %s\n",hostName, PORT_NO);
			} else {
				printf("\nEnter hostname of server you are trying to connect to:\n");
				scanf("%s", hostName);
				printf("Client connecting to %s......\n", hostName);

					activeConnection = serverConnect(&serverDetails,&serverInfo,&addressInfoPointer,hostName,PORT_NO);

				if (activeConnection > 0) {
					printf("Connection to Chat server is established. The server is running on %s at port %s\n\n",hostName, PORT_NO);
				} else {
					fprintf(stderr,"Failed to reach the given server %s. Please check the hostname you are trying to connect.",hostName);
				}
			}
		} else if (strcmp(command, "BEGIN") == 0) {
			if (activeConnection > 0) {
				struct datapacket dataPacket;
					if (createPacketForSending(command, &dataPacket) == -1) {
					fprintf(stderr, "Packet Creation Failed.\n\n");
				} else {
					sendDataPacket(&dataPacket);
					chatting = 1;
				}
				memset(&dataPacket, 0, sizeof(struct datapacket));
			} else {
				printf("CONNECT to the Chat Server first.\n\n");
			}
		} else if (strcmp(command, "TRANSFER") == 0) {
			if (activeConnection > 0 && chatting) {
				struct datapacket dataPacket;

				int generatedPacket = createPacketForSending(command,
						&dataPacket);
				if (generatedPacket == -1) {
					fprintf(stderr,
							"Error occurred during packet creation. Try again.\n");
				} else if (generatedPacket == -2) {
					printf("You do not have chat partner yet\n");
				} else {
					sendFileData(&dataPacket);
				}

				memset(&dataPacket, 0, sizeof(struct datapacket));
			} else if (activeConnection > 0 && !chatting) {
				printf("You need to enter the BEGIN command first .\n\n");
			} else {
				printf("CONNECT to the Chat Server first.");
			}
		}

		else if (strcmp(command, "QUIT") == 0) {
			if (activeConnection > 0) {
				struct datapacket dataPacket;

				if (createPacketForSending(command, &dataPacket) == -1) {
					fprintf(stderr, "Error occurred during packet creation.\n");
				} else {
					sendDataPacket(&dataPacket);
					chatting = 0;
					paired = 0;
				}

				memset(&dataPacket, 0, sizeof(struct datapacket));
			} else {
				printf("CONNECT to the Chat Server first.");
			}
		} else if (strcmp(command, "EXIT") == 0) {
			printf("Exiting chat\n");
			break;
		} else if (strcmp(command, "HELP") == 0) {
			if (activeConnection > 0) {
				struct datapacket dataPacket;
				if (createPacketForSending(command, &dataPacket) == -1) {
					fprintf(stderr, "Error occurred during packet creation.\n");
				} else {
					sendDataPacket(&dataPacket);
				}
				memset(&dataPacket, 0, sizeof(struct datapacket));
			} else {
				printf("Connection to server is not established.\n\n");
			}
		} else {
			strcpy(command, "CHAT");
			struct datapacket dataPacket;
			int return_genpacket = createPacketForSending(command, &dataPacket);
			if (return_genpacket == -1) {
				fprintf(stderr, "Error occurred during packet creation.\n");
			} else if (return_genpacket == -2) {
				printf("You do not have chat partner yet\n");
			} else {
				sendDataPacket(&dataPacket);
			}
			memset(&dataPacket, 0, sizeof(struct datapacket));
		}
	}

	close(fdSocket);
	printf("Client is closed.\n");
	return 0;
}
