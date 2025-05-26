# Tic-Tac-Toe Multiplayer (TCP & UDP)  
Implements a **two-player Tic-Tac-Toe game** over a network, supporting both **TCP** and **UDP** protocols. Players connect to a server and play interactively from separate clients.

---

## Overview

**Server:**  
The server will manage the game state (the Tic-Tac-Toe board), handle communication between the two players, and determine if there’s a winner or if the game ends in a draw.

**Clients:**  
Two clients will connect to the server and send their moves. The server will update the game board and broadcast the updated game state to both players.

---

## How to Compile

Open a terminal in the respective folder (`TCP` or `UDP`) and run:

**TCP:**
```
gcc server.c -o server
gcc client.c -o client
```

**UDP:**
```
gcc server.c -o server
gcc client.c -o client
```

---

## How to Run

### 1. Start the Server

In one terminal:
```
./server
```

### 2. Start the Clients

In two separate terminals (for each player):
```
./client
```

Both clients must connect to the server running on `127.0.0.1:8080`.

---

## Gameplay Instructions

- When prompted, **enter 'Ready'** to start.
- On your turn, **enter your move as two numbers**:  
  ```
  <row> <column>
  ```
  (Rows and columns are 1-based, e.g., `2 3` for row 2, column 3.)
- If you try to move out of turn, your move is buffered and will be processed when it's your turn.
- After the game ends, you'll be asked if you want to play again (`yes` or `no`).

---

## Implementation Notes

- **Turn Handling :**  
  If a player tries to make a move when it's not their turn, the move is **kept in the buffer** and processed when their turn arrives.  

- **TCP vs UDP:**  
  - TCP version uses `send`/`recv` for communication.
  - UDP version uses `sendto`/`recvfrom` and handles client addresses explicitly.

---

## Files

- [`TCP/server.c`](TCP/server.c): TCP server for Tic-Tac-Toe.
- [`TCP/client.c`](TCP/client.c): TCP client for Tic-Tac-Toe.
- [`UDP/server.c`](UDP/server.c): UDP server for Tic-Tac-Toe.
- [`UDP/client.c`](UDP/client.c): UDP client for Tic-Tac-Toe.

---


### Game Rules

**The Board:**
- The tic-tac-toe board is a 3x3 grid.
- Players take turns to place their symbol (‘X’ or ‘O’) in an empty spot.
- The game begins with an empty board, and the server will send this initial state to both players once the game starts.

**Starting the Game:**
- The game will start when both players (clients) have connected to the server and confirmed they are ready to play.
- The server will assign Player 1 to use ‘X’ and Player 2 to use ‘O’.

**Winning and Drawing:**
- A player wins if they successfully place three of their symbols (‘X’ or ‘O’) in a row, column, or diagonal.
- If the grid is full and no player has won, the game ends in a draw.
- After a win or a draw, the server informs both players of the outcome, displaying the final board and the appropriate message:
  - “Player 1 Wins!”
  - “Player 2 Wins!”
  - “It’s a Draw!”

**After the Game:**
- Once the game ends, both players are asked if they would like to play again.
- If both say yes, the server will reset the board and start a new game.
- If both say no, the server closes the connection for both.
- If one player says yes and the other says no, the player who wanted to continue is informed that their opponent did not wish to play, and the connection for both is closed.
