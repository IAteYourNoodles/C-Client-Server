// chat_client
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define host "localhost"

int main(int argc, char *argv[]) {
	// Receive port from an argument
	if (argc != 2) {
		printf("Incorrect number of arguments. Please enter port number.\n");
		return 1;
	}
	int port = atoi(argv[1]);

	// Create socket
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("socket failure");
		return 1;
	}

	// Get server host by name
	struct hostent* server = gethostbyname(host);	
	if (!server) {
		perror("server host");
		return 1;
	}
	if (server->h_addrtype != AF_INET) {
		perror("bad address family");
		return 1;
	}

	// Set up server address
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
        saddr.sin_addr.s_addr = ((struct in_addr*) server->h_addr_list[0])->s_addr;
        saddr.sin_port = htons(port);

	// Connect to server
	if (connect(sock_fd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0) {
		perror("connection failure");
		return 1;
	}

	// Read username from user
	char username[16];
	printf("Enter your username: ");
	fgets(username, sizeof(username), stdin);
	// Clean up username by removing newline
	username[strcspn(username, "\n")] = '\0';
	// Write username to server
	if (write(sock_fd, username, strlen(username)) < 0) {
		perror("writing to server error");
		return 1;
	}

	// Loop
	char read_buffer[256];
	char send_buffer[256];
	while (1) {
		// Clear send_buffer
		memset(send_buffer, 0, sizeof(send_buffer));

		// Get message from user
		printf("Send message (enter 'exit' to quit): ");
		fgets(send_buffer, sizeof(send_buffer), stdin);
		// Clean up new line
		send_buffer[strcspn(send_buffer, "\n")] = '\0';

		// Write to server
		//write(sock_fd, username, strlen(username));
		if (write(sock_fd, send_buffer, strlen(send_buffer)) < 0) {
			perror("writing to server error");
			continue;
		}

		// Exit case
		if (strcmp(send_buffer, "exit") == 0) {
			printf("Exiting...\n");
			break;
		}

		// Reset read_buffer
		memset(read_buffer, 0, sizeof(read_buffer));

		// Read and print server response
		if (read(sock_fd, read_buffer, sizeof(read_buffer)) < 0) {
			perror("reading from server error");
			continue;
		}
		printf("%s\n", read_buffer);
	}

	close(sock_fd);
	return 0;

}
