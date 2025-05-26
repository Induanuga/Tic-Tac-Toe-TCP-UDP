#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 2
#define PORT 8080
#define MAX_LEN 1024

int board[3][3];
int curr_player = 0;

struct sockaddr_in client_addr[MAX_CLIENTS];
socklen_t client_addr_len[MAX_CLIENTS];

void init_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = -1;
        }
    }
}

void display_board(int sockfd) {
    char buff[MAX_LEN];
    bzero(buff, sizeof(buff));
    snprintf(buff, sizeof(buff), "\nBoard:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == 0) {
                snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), " X ");
            } 
            else if (board[i][j] == 1) {
                snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), " O ");
            } 
            else {
                snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), " . ");
            }
            if (j < 2)
                snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), "|");
        }
        snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), "\n");
        if (i < 2) {
            snprintf(buff + strlen(buff), sizeof(buff) - strlen(buff), "-----------\n");
        }
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[i], client_addr_len[i]);
    }
}

int check_winner() {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != -1 && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return board[i][0];
        if (board[0][i] != -1 && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return board[0][i];
    }
    if (board[0][0] != -1 && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return board[0][0];
    if (board[0][2] != -1 && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return board[0][2];
    return -1;
}

int check_draw() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == -1)
                return 0;
        }
    }
    return 1;
}

void handle_client(int sockfd) {
    char buff[MAX_LEN];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sprintf(buff, "Player %d, enter 'Ready' when you are ready to start: ", i + 1);
        sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[i], client_addr_len[i]);
        while (1) {
            bzero(buff, sizeof(buff));
            recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr[i], &client_addr_len[i]);
            if (strcmp(buff, "Ready") == 0) {
                sprintf(buff, "Player %d is ready!\n", i + 1);
                sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[0], client_addr_len[0]);
                sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[1], client_addr_len[1]);
                break;
            } else {
                sprintf(buff, "Please enter 'Ready' to start the game.\n");
                sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[i], client_addr_len[i]);
            }
        }
    }

    sprintf(buff, "Both players are ready! Let's start the game.\nPlayer 1 is X and Player 2 is O\n");
    sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[0], client_addr_len[0]);
    sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[1], client_addr_len[1]);

loc:
    while (1) {
        display_board(sockfd);
        int r, c;
        while (1) {
            bzero(buff, sizeof(buff));
            sprintf(buff, "(Player %d's turn) Enter row and column: ", curr_player + 1);
            sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[curr_player], client_addr_len[curr_player]);
            bzero(buff, sizeof(buff));
            recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr[curr_player], &client_addr_len[curr_player]);
            sscanf(buff, "%d %d", &r, &c);
            r--;
            c--;
            if (r >= 0 && r < 3 && c >= 0 && c < 3 && board[r][c] == -1) {
                board[r][c] = curr_player;
                break;
            } 
            else {
                sprintf(buff, "Invalid move! Try again.\n");
                sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[curr_player], client_addr_len[curr_player]);
            }
        }

        int winner = check_winner();
        if (winner != -1) {
            display_board(sockfd);
            sprintf(buff, "Player %d wins!\n", winner + 1);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[i], client_addr_len[i]);
            }
            break;
        }
        if (check_draw()) {
            display_board(sockfd);
            sprintf(buff, "It's a draw!\n");
            for (int i = 0; i < MAX_CLIENTS; i++) {
                sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[i], client_addr_len[i]);
            }
            break;
        }
        curr_player = 1 - curr_player;
    }

    int playAgain[2] = {0, 0};
    for (int i = 0; i < MAX_CLIENTS; i++) {
        bzero(buff, sizeof(buff));
        sprintf(buff, "Player %d, do you want to play again? (yes/no): ", i + 1);
        sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[i], client_addr_len[i]);
        bzero(buff, sizeof(buff));
        recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&client_addr[i], &client_addr_len[i]);
        buff[strcspn(buff, "\n")] = 0;
        if (strcmp(buff, "yes") == 0) {
            playAgain[i] = 1;
        } 
        else if (strcmp(buff, "no") == 0) {
            playAgain[i] = 0;\
        }
    }

    if (playAgain[0] == 1 && playAgain[1] == 1) {
        init_board();
        curr_player = 0; 
        goto loc;
    } 
    else if (playAgain[0] == 0 && playAgain[1] == 0) {
        bzero(buff, sizeof(buff));
        sprintf(buff, "Both players said no. Closing the connection.\n");
        sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[0], client_addr_len[0]);
        sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[1], client_addr_len[1]);
        close(sockfd);
        return;
    } 
    else {
        if (playAgain[0] == 1 && playAgain[1] == 0) {
            bzero(buff, sizeof(buff));
            sprintf(buff, "Player %d does not want to play again. Closing the connection.\n", 2);
            sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[0], client_addr_len[0]);
            sprintf(buff, "You chose not to continue. Closing the connection.\n");
            sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[1], client_addr_len[1]);
        }
        else if (playAgain[0] == 0 && playAgain[1] == 1) {
            bzero(buff, sizeof(buff));
            sprintf(buff, "Player %d does not want to play again. Closing the connection.\n", 1);
            sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[1], client_addr_len[1]);
            sprintf(buff, "You chose not to continue. Closing the connection.\n");
            sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&client_addr[0], client_addr_len[0]);
        }
        close(sockfd);
        return;
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket successfully created..\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    }
    printf("Socket successfully binded..\n");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_addr_len[i] = sizeof(client_addr[i]);
        recvfrom(sockfd, NULL, 0, 0, (struct sockaddr *)&client_addr[i], &client_addr_len[i]);
        printf("Client %d connected...\n", i + 1);
    }

    init_board();
    handle_client(sockfd);

    close(sockfd);
    return 0;
}
