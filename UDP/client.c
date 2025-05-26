#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_MSG_LEN 1024


void func(int sockfd, struct sockaddr_in *servaddr) {
    char buffer[MAX_MSG_LEN];
    socklen_t addr_len = sizeof(*servaddr);
    
    while (1) {
        bzero(buffer, sizeof(buffer));
        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)servaddr, &addr_len);
        if (n <= 0) {
            printf("Connection closed by server.\n");
            break;
        }
        buffer[n] = '\0';
        printf("%s", buffer);
        if (strstr(buffer, "Closing the connection") != NULL) {
            break;
        }
        if (strstr(buffer, "enter 'Ready'") != NULL) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)servaddr, addr_len); 
        }
        else if (strstr(buffer, "Enter row and column") != NULL) {
            printf("(Format: <row column>): ");
            fgets(buffer, sizeof(buffer), stdin);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)servaddr, addr_len); 
        }
        else if (strstr(buffer, "do you want to play again?") != NULL) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0; 
            while (strcmp(buffer, "yes") != 0 && strcmp(buffer, "no") != 0) {
                printf("Invalid response. Please enter 'yes' or 'no': ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0; 
            }
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)servaddr, addr_len);
        }
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    const char *msg = "Hello from client";
    sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    func(sockfd, &servaddr);
    close(sockfd);
    return 0;
}
