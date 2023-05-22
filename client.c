#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#define SERVER_ADDRESS "localhost"
#define SERVER_PORT 9034
#define MAX_BUFFER_SIZE 256

int main(void)
{
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &(serverAddr.sin_addr));

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect");
        exit(1);
    }

    printf("Connected to the server. You can start typing...\n");

    struct pollfd fds[2];
    fds[0].fd = sockfd; // Socket for receiving data from the server
    fds[0].events = POLLIN; // Check ready-to-read
    fds[1].fd = fileno(stdin); // Standard input for reading user input
    fds[1].events = POLLIN; // Check ready-to-read

    while (1) {
        // Poll for events
        int pollCount = poll(fds, 2, -1);
        if (pollCount == -1) {
            perror("poll");
            exit(1);
        }

        // Check if data is available from the server
        if (fds[0].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            int numBytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (numBytes == -1) {
                perror("recv");
                exit(1);
            } else if (numBytes == 0) {
                printf("Server closed the connection.\n");
                break;
            }
            printf("Received: %s\n", buffer);
        }

        // Check if user input is available
        if (fds[1].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                perror("fgets");
                exit(1);
            }
            buffer[strcspn(buffer, "\n")] = '\0';

            // Check if the user wants to exit
            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            // Send data to the server
            if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
                perror("send");
                exit(1);
            }
        }
    }

    // Close the socket
    close(sockfd);

    return 0;
}
