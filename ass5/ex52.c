/*
 * Lee Alima
 * 313467441
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define BOARD_SIZE 20
#define LEFT 'a'
#define RIGHT 'd'
#define DOWN 's'
#define FLIP 'w'
#define DONE 'q'

// saves info about the shape of the player
enum State {Stand,Lay};

// saves date about the board game
typedef struct Board{
    char board[BOARD_SIZE][BOARD_SIZE];
    int middle_row;
    int middle_col;
    enum State state;
} Board;

// global variable
Board board;

void play();
void fillBoard();
void printBoard();
void updateBoard();
void moveLeft();
void moveRight();
void moveDown();
void moveFlip();
void handleSignal();
void alarmMoveDown();

int main() {
    play();
}

/**
 * This function creates a new lay player in the upper middle point
 * of the board, fills the board and handles signals - SIGUSR2 and SIGALRM
 */
void play(){
    // creates a new player
    board.middle_row = 0;
    board.middle_col = BOARD_SIZE/2;
    board.state = Lay;
    fillBoard(board);
    printBoard();
    // listen to signal SIGALRM
    signal(SIGALRM, alarmMoveDown);
    alarm(1);
    // listen to signal SIGUSR2
    signal(SIGUSR2, handleSignal);
    while (1) { //keep running
        pause();
    }
}

/**
 * This funcion initialize the board an fill all the
 * cells as was asked in the Targil
 */
void fillBoard(){
    int i,j;
    for (i = 0 ; i<BOARD_SIZE ; i++){
        for(j = 0 ; j<BOARD_SIZE ; j++){
            if (i == BOARD_SIZE - 1) { // part of the frame
                board.board[i][j] = '*';
            } else if(j == 0 || j == BOARD_SIZE - 1) { // part of the fram
                board.board[i][j] = '*';
            } else{ // empty cells
                board.board[i][j] = ' ';
            }
        }
    }
}

/**
 * This function prints the board
 */
void printBoard(){
    updateBoard();
    // clearing the screen (other boards)
    system("clear");
    int i,j;
    for (i = 0 ; i<BOARD_SIZE ; i++){
        for(j = 0 ; j<BOARD_SIZE ; j++){
            printf("%c", board.board[i][j]);
        }
        printf("\n"); // next line
    }
}

/**
 * This function changes the cells that represents the player
 * to -
 */
void updateBoard(){
    if (board.state == Lay){
        board.board[board.middle_row][board.middle_col-1] = '-';
        board.board[board.middle_row][board.middle_col] = '-';
        board.board[board.middle_row][board.middle_col+1] = '-';
    } else {
        board.board[board.middle_row-1][board.middle_col] = '-';
        board.board[board.middle_row][board.middle_col] = '-';
        board.board[board.middle_row+1][board.middle_col] = '-';
    }
}

/**
 * This funcion moves the player left
 */
void moveLeft() {
    if (board.state == Stand){ // Stand
        if (board.middle_col > 1){
            board.middle_col-=1;
        }
    }else { // Lay
        if (board.middle_col > 2){
            board.middle_col-=1;
        }
    }
}

/**
 * This funcion moves the player right
 */
void moveRight() {
    if (board.state == Stand){ // Stand
        if (board.middle_col < BOARD_SIZE-2){
            board.middle_col+=1;
        }
    }else { // Lay
        if (board.middle_col < BOARD_SIZE -3 ){
            board.middle_col+=1;
        }
    }
}

/**
 * This funcion moves the player down
 */
void moveDown() {
    // if player should be cleared
    if ((board.state == Lay && board.middle_row >= BOARD_SIZE-2)
        || (board.state == Stand && board.middle_row  >= BOARD_SIZE -3)){
        board.state = Lay;
        board.middle_row = 0;
        board.middle_col = BOARD_SIZE/2;
    }
    else if (board.state == Stand){ // Stand
        if (board.middle_row < BOARD_SIZE-3){
            board.middle_row+=1;
        }
    }else { // Lay
        if (board.middle_row < BOARD_SIZE -2 ){
            board.middle_row+=1;
        }
    }
}

/**
 * This function flips the player shape
 */
void moveFlip() {
    if (board.state == Stand){
        if (board.middle_col < BOARD_SIZE -2 && board.middle_col > 1){ // can flip
            board.state = Lay;
        }
    } else {
        if (board.middle_row < BOARD_SIZE-2 && board.middle_row > 0){ // can flip
            board.state = Stand;
        }
    }
}

/**
 * This function clear the player '-' cells from the board
 */
void clearShape(){
    if (board.state == Stand){ // Stand
        board.board[board.middle_row-1][board.middle_col] = ' ';
        board.board[board.middle_row][board.middle_col] = ' ';
        board.board[board.middle_row+1][board.middle_col] = ' ';
    } else { // Lay
        board.board[board.middle_row][board.middle_col-1] = ' ';
        board.board[board.middle_row][board.middle_col] = ' ';
        board.board[board.middle_row][board.middle_col+1] = ' ';
    }
}

/**
 * This function handle signals from ex51.c -
 * reading chars from the pipe instead of stdin
 */
void handleSignal() {
    char c;
    scanf("%c", &c); // get char from pipe
    clearShape();
    switch(c) {
        case LEFT:
            moveLeft();
            break;
        case RIGHT:
            moveRight();
            break;
        case DOWN:
            moveDown();
            break;
        case FLIP:
            moveFlip();
            break;
        case DONE:
            exit(0);
    }
    printBoard();
    // get next signal
    signal(SIGUSR2,handleSignal);
}

/**
 * This function is called every 1 second and calls moveDown
 */
void alarmMoveDown(){
    signal(SIGALRM, alarmMoveDown); // listen to SIGALRM
    alarm(1); // next moving down
    clearShape();
    moveDown();
    printBoard();
}

