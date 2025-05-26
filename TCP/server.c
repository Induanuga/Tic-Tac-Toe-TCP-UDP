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


void init_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = -1;
        }
    }
}

void display_board(int sockfd1, int sockfd2) {
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
    send(sockfd1, buff, strlen(buff), 0);
    send(sockfd2, buff, strlen(buff), 0);
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


void handle_client(int client_sock_fds[]) {
    char buff[MAX_LEN];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sprintf(buff, "Player %d, enter 'Ready' when you are ready to start: ", i + 1);
        send(client_sock_fds[i], buff, strlen(buff), 0);
        bzero(buff, sizeof(buff));
        while (1) {
            bzero(buff, sizeof(buff));
            recv(client_sock_fds[i], buff, sizeof(buff), 0);
            int len = strlen(buff);
            if (buff[len - 1] == '\n') {
                buff[len - 1] = '\0';
            }
            if (strcmp(buff, "Ready") == 0) {
                bzero(buff, sizeof(buff));
                sprintf(buff, "Player %d is ready!\n", i + 1);
                send(client_sock_fds[0], buff, strlen(buff), 0);
                send(client_sock_fds[1], buff, strlen(buff), 0);
                break;
            } 
            else {
                bzero(buff, sizeof(buff));
                sprintf(buff, "Please enter 'Ready' to start the game.\n");
                send(client_sock_fds[i], buff, strlen(buff), 0);
            }
        }
    }
    bzero(buff, sizeof(buff));
    sprintf(buff, "Both players are ready! Let's start the game.\nPlayer 1 is X and Player 2 is O\n");
    send(client_sock_fds[0], buff, strlen(buff), 0);
    send(client_sock_fds[1], buff, strlen(buff), 0);
loc:
    while (1) {
        display_board(client_sock_fds[0], client_sock_fds[1]);
        sleep(1);
        int r, c;
        while (1) {
            bzero(buff, sizeof(buff));
            sprintf(buff, "(Player %d's turn) Enter row and column ", curr_player + 1);
            send(client_sock_fds[curr_player], buff, strlen(buff), 0);
            bzero(buff, sizeof(buff));
            recv(client_sock_fds[curr_player], buff, sizeof(buff), 0);
            sscanf(buff, "%d %d", &r, &c);
            r--;
            c--;
            if (r >= 0 && r < 3 && c >= 0 && c < 3 && board[r][c] == -1) {
                board[r][c] = curr_player;
                break; 
            } 
            else {
                bzero(buff, sizeof(buff));
                sprintf(buff, "Invalid move! Try again.\n");
                send(client_sock_fds[curr_player], buff, strlen(buff), 0);
            }
        }

        int winner = check_winner();
        if (winner != -1) {
            display_board(client_sock_fds[0], client_sock_fds[1]);
            bzero(buff, sizeof(buff));
            sprintf(buff, "Player %d wins!\n", winner + 1);
            send(client_sock_fds[0], buff, strlen(buff), 0);
            send(client_sock_fds[1], buff, strlen(buff), 0);
            break;
        }
        if (check_draw()) {
            display_board(client_sock_fds[0], client_sock_fds[1]);
            bzero(buff, sizeof(buff));
            sprintf(buff, "It's a draw!\n");
            send(client_sock_fds[0], buff, strlen(buff), 0);
            send(client_sock_fds[1], buff, strlen(buff), 0);
            break;
        }
        curr_player = 1 - curr_player;
    }

    sleep(1);

    int playAgain[2] = {0, 0};
    for (int i = 0; i < MAX_CLIENTS; i++) {
        bzero(buff, sizeof(buff));
        sprintf(buff, "Player %d, do you want to play again? (yes/no): ", i + 1);
        send(client_sock_fds[i], buff, strlen(buff), 0);
        bzero(buff, sizeof(buff));
        recv(client_sock_fds[i], buff, sizeof(buff), 0);
        buff[strcspn(buff, "\n")] = 0;
        if (strcmp(buff, "yes") == 0) {
            playAgain[i] = 1;
        } 
        else if (strcmp(buff, "no") == 0) {
            playAgain[i] = 0;
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
        send(client_sock_fds[0], buff, strlen(buff), 0);
        send(client_sock_fds[1], buff, strlen(buff), 0);
        close(client_sock_fds[0]);
        close(client_sock_fds[1]);
        return;
    } 
    else {
        if(playAgain[0]==1 && playAgain[1]==0){
            bzero(buff, sizeof(buff));
            sprintf(buff, "Player %d does not want to play again. Closing the connection.\n", 2);
            send(client_sock_fds[0], buff, strlen(buff), 0);
        }
        else if(playAgain[0]==0 && playAgain[1]==1){
            bzero(buff, sizeof(buff));
            sprintf(buff, "Player %d does not want to play again. Closing the connection.\n", 1);
            send(client_sock_fds[1], buff, strlen(buff), 0);
        }
        close(client_sock_fds[0]);
        close(client_sock_fds[1]);
        return;
    }
}



int main() {
    int server_sockfd, client_sock_fds[MAX_CLIENTS], len;
    struct sockaddr_in server_addr, client_addr;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket successfully created..\n");
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        printf("Socket bind failed...\n");
        exit(0);
    }
    printf("Socket successfully binded..\n");

    if (listen(server_sockfd, MAX_CLIENTS) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    printf("Server listening..\n");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        len = sizeof(client_addr);
        client_sock_fds[i] = accept(server_sockfd, (struct sockaddr *)&client_addr, &len);
        if (client_sock_fds[i] < 0) {
            printf("Server accept failed...\n");
            exit(0);
        }
        printf("Server accepted client %d...\n", i + 1);
    }

    init_board();
    handle_client(client_sock_fds);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        close(client_sock_fds[i]);
    }
    close(server_sockfd);
    return 0;
}
