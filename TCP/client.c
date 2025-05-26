#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_MSG_LEN 1024


void func(int sockfd) {
    char buffer[MAX_MSG_LEN];
    while (1) {
        bzero(buffer, sizeof(buffer));
        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("Connection closed by server.\n");
            break;
        }
        buffer[n] = '\0';
        printf("%s", buffer);
        if (strstr(buffer, "enter 'Ready'") != NULL) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            write(sockfd, buffer, strlen(buffer));
        }
        else if (strstr(buffer, "Enter row and column") != NULL) {
            printf("(Format: <row column>): ");
            fgets(buffer, sizeof(buffer), stdin);
            write(sockfd, buffer, strlen(buffer));
        }
        else if (strstr(buffer, "do you want to play again?") != NULL) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            while (strcmp(buffer, "yes") != 0 && strcmp(buffer, "no") != 0) {
                printf("Invalid response. Please enter 'yes' or 'no': ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
            }
            write(sockfd, buffer, strlen(buffer));
        }
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection with the server failed...\n");
        exit(0);
    }
    printf("Connected to the server..\n");
    func(sockfd);
    close(sockfd);
    return 0;
}
