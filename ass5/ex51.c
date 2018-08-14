/*
 * Lee Alima
 * 313467441
 */

#include <termio.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define FAIL -1
#define STDERR 2
#define LENGTH_ERROR_MSG 21
#define END_GAME 'q'
#define PROG "./draw.out"

void handleKeyboard();


int main() {
    handleKeyboard();
}

/**
 * This function was given and wasn't implemented by me
 * @return char - the user typed
 */
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}

/**
 * This function is being called in case of an error,
 * I wrote a msg for the user and exit the program
 */
void handleFailure(){
    write(STDERR,"Error in system call\n",LENGTH_ERROR_MSG);
    exit(FAIL);
}

/**
 * This function is called from the main function, Here I created
 * a child that will execute the program created in ex52.c
 * that gets input using pipe and not the normal stdin
 */
void handleKeyboard(){
    int Pipe[2];
    if (pipe(Pipe) == FAIL){ // pipe failed
        handleFailure();
    }
    int pid = fork();
    if (pid < 0){ // fork failed
        handleFailure();
    }
    if (pid == 0){ // for child
        dup2(Pipe[0], STDIN_FILENO); // close STDIN_FILENO and use Pipe[0]
        // for reading
        char* args[2] = {PROG, NULL}; // args for execution
        execvp(args[0],args); // execute
        handleFailure();
    } else { // father - handling typing
        char t;
        // if flag is zero it means the prog should continue working,
        // and if it's 0 it means than the user asked to quit
        int flag = 1;
        while (flag){
            t = getch();
            // ignore those chars
            if (t != 'a' && t != 's' && t != 'd' && t != 'w' && t != 'q'){
                continue;
            } // if the char is matter (from the given list)
            if (write(Pipe[1],&t,sizeof(char))<0){ // write the char throughout the pipe
                handleFailure();
            }
            kill(pid,SIGUSR2); // send signal SIGUSR2
            if (t == END_GAME){
                flag = 0;
                close(Pipe[0]);
                close(Pipe[1]);
            }
        }
    }
}