// chat_server.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 3
#define BUFFER_SIZE 256

struct ClientInfo {
	int client_fd;
	char username[16];
	int active; 
};

struct ClientInfo client_infos[MAX_CLIENTS];

// Thread for each client connected
void *handle_client(void *arg) {
	struct ClientInfo *client_info = (struct ClientInfo *)arg;
	int client_fd = client_info->client_fd;
	char buffer[BUFFER_SIZE];

	// Write join to chat_history
	FILE *history = fopen("chat_history", "a");
	if (history == NULL) {
		perror("unable to open file");
		return NULL;
	}
	fprintf(history, "%s has entered the chat.\n", client_info->username);
	printf("%s has entered the chat.\n", client_info->username);
	fclose(history);

	while (client_info->active) {
		// Clear buffer
		memset(buffer, 0, sizeof(buffer));

                // Read client message
                ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer));
                if (bytesRead < 0) {
                        perror("error reading from socket");
                        continue;
                } else if (bytesRead == 0) {
			// Client closed the connection
			client_info->active = 0;
			break;
		}

		// Open to chat_history
		history = fopen("chat_history", "a");
		if (history == NULL) {
			perror("unable to open file");
			return NULL;
		}

		// Check for user exit
		if (strcmp(buffer, "exit") == 0) {
                        printf("%s has left the chat.\n", client_info->username);
                        fprintf(history, "%s has left the chat.\n", client_info->username);
			fclose(history);
			client_info->active = 0;
                        break;
                }

		// Write to chat_history and close
		fprintf(history, "%s: %s\n", client_info->username, buffer);
		printf("%s: %s\n", client_info->username, buffer);
		fclose(history);

		// Broadcast to all clients
		char broadcast_message[BUFFER_SIZE+32];
		snprintf(broadcast_message, sizeof(broadcast_message), "%s: %s\n", client_info->username, buffer);
		for (int i = 0; i < MAX_CLIENTS; ++i) {
         		struct ClientInfo *other_client = &client_infos[i];
			// Write to other clients that are active
         		if  (other_client->active) {
                		if (write(other_client->client_fd, broadcast_message, strlen(broadcast_message)) < 0) {
					perror("error writing to socket");
                		}
            		}
        	}
	}
	close(client_fd);
	return NULL;
}

int main(int argc, char *argv[]) {
	// Receive port as an argument
	if (argc != 2) {
		printf("Incorrect arguments passed. Please enter port number.\n");
		return 1;	
	}
	int port = atoi(argv[1]);

	// Create socket
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("socket failure");
		return 1;
	}

	// Set up server address
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(port);

	// Bind socket
	if (bind(sock_fd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0) {
		perror("bind failure");
		close(sock_fd);
		return 1;
	}

	// Listen for connections
	if (listen(sock_fd, MAX_CLIENTS) < 0) {
		perror("listen failure");
		close(sock_fd);
		return 1;
	}

	printf("Server listening on port %d...\n", port);

	int client_fd;
	int i = 0;
	while (1) {
		struct sockaddr_in caddr; // client address
		socklen_t len = sizeof(caddr);
		
		// Accept connection
		client_fd = accept(sock_fd, (struct sockaddr*) &caddr, &len);
		if (client_fd < 0) {
			perror("accept failure");
			continue;
		}

		// Create a struct for each client
		struct ClientInfo *client_info = &client_infos[i];
		i++;
		
		// Setup ClientInfo and read username
		client_info->client_fd = client_fd;
		client_info->active = 1;
		memset(client_info->username, 0, sizeof(client_info->username));
		read(client_fd, client_info->username, sizeof(client_info->username));

		// Thread creation
		pthread_t t_id;
		if (pthread_create(&t_id, NULL, handle_client, client_info) != 0) {
			perror("thread creation failure");
			close(client_fd);
			continue;
		}

		// Thread detachment
		pthread_detach(t_id);
	}

	close(sock_fd);
	return 0;
}
