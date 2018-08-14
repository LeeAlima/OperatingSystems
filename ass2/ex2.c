// 313467441 Lee alima

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INPUT_SIZE 512
#define NUMBER_OF_WORDS 32
#define NUMBER_OF_JOBS 32

typedef struct job{
    pid_t pid;
    char* commandArgs;
} job;

void shellLoop(void);
char *getInput();
int executeCommand(char *args, struct job **jobs,int *numOfJobs);
void handleCdCommand(char *command);
void handleJobsCommand(struct job **jobs,int *numOfJobs);
void handleCommand(char *args,int *numOfJobs,struct job **jobs);
int checkLastArg(char **args);
void updateJobs(struct job **jobs,int *numOfJobs);
char **cutInput(char *command);
void freeJobs(struct job** jobs,int *numOfJobs);
void handleFailingAllocation(char *command,char *copy, char **args,
                             struct job** jobs,int *numOfJobs);
/**
 * This is the main function of the project.
 */
int main(int argc, char **argv)
{
    shellLoop();
}

/**
 * This function is being called from the main function,
 * In this function, the user is being asked for input (command),
 * and the input is send to a function that will handle it.
 * At the end of the function, I freed all the jobs that has't been finished.
 */
void shellLoop(void)
{
    char *command;
    int status;
    int numOfJobs = 0;
    struct job **jobs = (struct job**)calloc(NUMBER_OF_JOBS,sizeof(struct job*));
    // check calloc
    if (!jobs){ // if malloc has failed, print an error msg and exit
        fprintf(stderr, "Error in system call\n");
        exit(1);
    }
    do {
        printf("prompt=");
        command = getInput(); // get the input from the user
        status = executeCommand(command,jobs,&numOfJobs); // execute the command
        free(command);
    } while (status);
    freeJobs(jobs,&numOfJobs);
}

/**
 * This function gets the input from the user, char by char.
 * @return char* - The user input.
 */
char *getInput(){
    int bufferSize = INPUT_SIZE;
    char *buffer = malloc(sizeof(char) * bufferSize);
    if (!buffer){
        fprintf(stderr, "Error in system call\n");
        exit(1);
    }
    char oneChar;
    int counter = 0;
    char *backup;
    while(1){
        oneChar = getchar(); // get only one char
        if (oneChar == '\n') { // if the user typed '\n'
            buffer[counter] = '\0'; // replace it by null (end of char*)
            return buffer;
        } else {
            buffer[counter] =  oneChar; // save the oneChar in the buffer
            counter++;
        }
        if (counter >= bufferSize){ // if there is no more place for the command
            bufferSize *=2;
            backup = realloc(buffer,bufferSize);
            if (!backup){ // if realloc has failed, exit.
                fprintf(stderr, "Error in system call\n");
                free(buffer);
                exit(1);
            }
            buffer = backup;
        }
    }
}

/**
 * This function cut the command by spaces, If there exist "\""
 * the function finds them and saves the words between them as one token.
 * @param command (char *)
 * @return array of char*
 */
char **cutInput(char *command){
    int bufferSize = NUMBER_OF_WORDS;
    int counter = 0;
    char **backup;
    char **tokens = malloc(sizeof(char*) * bufferSize);
    if (!tokens){ // check malloc
        fprintf(stderr, "Error in system call\n");
        exit(1);
    }
    int quotStr = (*command == '\"');
    char* currerntQuot = strtok(command, "\"");
    char* next = NULL;
    // if it's not the end of the command.
    while(currerntQuot)
    {
        if(quotStr) // if "was found.
        {
            tokens[counter] = currerntQuot; // save it as one token
            quotStr = 0;
            counter++;
            if (counter >= bufferSize){
                bufferSize *=2;
                backup = realloc(tokens,bufferSize * sizeof(char*));
                if (!backup){ // check realloc
                    fprintf(stderr, "Error in system call\n");
                    free(tokens);
                    exit(1);
                }
                tokens =backup;
            }
        }
        else // if " wasn't found
        {
            next = strtok(next, "\0");
            // split by space
            char* currWord = strtok(currerntQuot, " ");
            while(currWord)
            {
                tokens[counter] = currWord;
                currWord = strtok(NULL, " ");
                counter++;
                if (counter >= bufferSize){
                    bufferSize *=2;
                    backup = realloc(tokens,bufferSize * sizeof(char*));
                    if (!backup){
                        fprintf(stderr, "Error in system call\n");
                        free(tokens);
                        exit(1);
                    }
                    tokens =backup;
                }
            }
            quotStr = 1;
        }
        currerntQuot = strtok(next, "\""); // try to find " for the next iteration
        next = NULL;
    }
    tokens[counter] = NULL;
    return tokens;
}

/**
 * This function gets the all input from the user,
 * by getting the first word in the command it calls the right function.
 * @param commandInput - char * - the all input from the user
 * @param jobs - the array of job*
 * @param numOfJobs - the number of running jobs
 * @return 0 if the command is exit and 1 otherwise.
 */
int executeCommand(char *commandInput, struct job **jobs,int *numOfJobs){
    int returnVal = 1;
    char *command = calloc(sizeof(char),strlen(commandInput)+1);
    if (!command){
        fprintf(stderr, "Error in system call\n");
        exit(1);
    }
    int counter = 0;
    // find the first space ot '\0'
    while (commandInput[counter] != ' ' && commandInput[counter] != '\0'){
        counter++;
    }
    // copy the first word to the command
    strncpy(command, commandInput,counter);
    // save a copy on the heap
    char *copy = malloc(sizeof(char)*strlen(commandInput)+1);
    if (!copy){
        fprintf(stderr, "Error in system call\n");
        free(command);
        exit(1);
    }
    strcpy(copy,commandInput);
    if (strcmp(command,"") == 0){
        // do nothing
    } else if (strcmp(command,"cd") == 0){
        handleCdCommand(copy);
    } else if (strcmp(command,"jobs") == 0){
        handleJobsCommand(jobs,numOfJobs);
    } else if (strcmp(command,"exit") == 0){
        returnVal = 0;
    } else {
        handleCommand(copy,numOfJobs,jobs);
    }
    free(command);
    free(copy);
    return returnVal;
}

/**
 * This function handle the "cd" command.
 * @param command - cd + path (char *)
 */
void handleCdCommand(char *command){
    static char path[256] =  "";
    char **args = cutInput(command);
    char copy[256];
    printf("%d\n",getpid()); // print the PID of the main process
    if (args[1] == NULL || strcmp(args[1],"~") == 0){ // no args -> go to HOME
        getcwd(path,sizeof(path));
        chdir(getenv("HOME"));
    } else if (strcmp(args[1],"-") == 0) {
        strcpy(copy,path);
        getcwd(path,sizeof(path));
        if (chdir(copy) != 0){
            fprintf(stderr, "OLDPWD not set\n");
            strcpy(path,copy);
        } else {
            printf("%s\n", copy);
        }
    } else {
        strcpy(copy,path);
        getcwd(path,sizeof(path));
        if (chdir(args[1]) != 0) { // try chdir with parameters
            fprintf(stderr, "Error in system call\n");
            strcpy(path,copy);
      }
    }
    free(args);
}

/**
 * This function prints all of the running jobs.
 * @param jobs - array of pointers to jobs
 * @param numOfJobs  - number of running jobs.
 */
void handleJobsCommand(struct job **jobs,int *numOfJobs){
    int counter = 0;
    int index = 0;
    // update the array of jobs - some jobs may be finished at this time.
    updateJobs(jobs,numOfJobs);
    /*
     * find all the unempty cells in the jobs array ( cells that
     * don't point to NULL ), and print their info.
     * The value of numOfJobs is the number of unempty cells.
     */
    while (counter != *numOfJobs){
        if (jobs[index] != NULL){
            counter++;
            printf("%d %s\n",jobs[index]->pid,jobs[index]->commandArgs);
        }
        index++;
    }
}

/**
 * This function handles the command asked from the user by
 * calling the built function execvp with the user Input
 * @param command - the user Input
 * @param numOfJobs - the number of jobs in the array
 * @param jobs - array of pointers to jobs
 */
void handleCommand(char *command,int *numOfJobs,struct job **jobs) {
    int size = NUMBER_OF_JOBS;
    char *copy = malloc(strlen(command) * sizeof(char)+1);
    if (!copy){
        fprintf(stderr, "Error in system call\n");
        exit(1);
    }
    strcpy(copy, command);
    char **args = cutInput(copy); // split command
    int isInBackround = checkLastArg(args); // check the last token
    if (isInBackround != -1) { // if the command ends in &
        args[isInBackround] = NULL; // put NULL at the last token
        char *find = strstr(command, " &"); // put NULL instead of the & in the command
        *find = '\0';
    }
    pid_t status, p = fork();
    if (p == 0) { // for son
        if (execvp(args[0], args) < 0) { // try to execute
            fprintf(stderr, "Error in system call\n");
            exit(1);
        }
    } else { // for father
        if (p < 0) {
            fprintf(stderr, "Error in system call\n");
            exit(1);
        } else {
            printf("%d\n", p); // print the son PID
            if (isInBackround == -1) { // wait for son
                waitpid(p, &status, 0);
            } else {
                job *newJob = calloc(1, sizeof(job));
                if (!newJob){
                    handleFailingAllocation(command,copy,args,jobs,numOfJobs);
                }
                // check for place in the array of jobs
                if (*numOfJobs >= size) { // jobs array should be reallocate
                    struct job **backup;
                    size *=2;
                    backup = realloc(jobs, size*sizeof(struct job*));
                    if (!backup) { // check realloc
                        handleFailingAllocation(command,copy,args,jobs,numOfJobs);
                    }
                    jobs = backup;
                }
                // initialize the newJob with values
                newJob->pid = p;
                newJob->commandArgs = malloc(sizeof(char *) * strlen(command)+1);
                if (!newJob->commandArgs){
                    free(newJob);
                    handleFailingAllocation(command,copy,args,jobs,numOfJobs);
                }
                strcpy(newJob->commandArgs, command);
                int counter = 0;
                updateJobs(jobs,numOfJobs); // update the array of jobs
                (*numOfJobs)++; // increase the number of running jobs
                while (1) { // find the first "empty" cell
                    if (jobs[counter] == NULL) {
                        jobs[counter] = newJob;
                        free(copy);
                        free(args);
                        return;
                    }
                    counter++;
                }
            }
        }
        free(copy);
        free(args);
    }
}

/**
 * This function checks if the last argument of the command is &
 * and the process should run at the backround
 * @param args - as the command splitted by spaces
 * @return -1 if the command doesn't end at & and the
 * index of & otherwise.
 */
int checkLastArg(char **args){
    char **ptr = args;
    char *lastInLoop;
    int counter = 0;
    while (*ptr != NULL){
        ptr++;
        counter++;
    }
    lastInLoop = args[counter-1];
    // check if the last char* of command is "&"
    return (strcmp(lastInLoop,"&") == 0? counter-1 :-1);
}

/**
 * This function updates the array of jobs, it goes over
 * all of the existed jobs and checks if they have ended or not.
 * If the ended it free the memory allocated and save NULL in the array.
 * @param jobs - array of pointers to jobs
 * @param numOfJobs - the number of jobs in the array
 */
void updateJobs(struct job **jobs,int *numOfJobs){
    struct job **copyJobs = jobs;
    struct job *ptr;
    int counter = *numOfJobs;
    // go over all of the jobs in the array using the counter
    while (counter != 0){
        ptr = *copyJobs;
        if (ptr != NULL){
            pid_t pid = ptr->pid;
            counter--;
            pid_t return_pid = waitpid(pid,NULL,WNOHANG);
            if (return_pid == -1){ // error
                fprintf(stderr, "Error in system call\n");
            } else if (return_pid == pid){
                (*numOfJobs)--; // reduce the number of existing jobs
                free(ptr->commandArgs);
                free(ptr);
                *copyJobs = NULL;
            }
        }
        copyJobs++;
    }
}

/**
 * This function prints an error msg to the user, frees all the
 * jobs and exit.
 * @param jobs - array of pointers to jobs
 * @param numOfJobs - the number of jobs in the array
 */
void handleFailingAllocation(char *command,char *copy, char **args,
                             struct job** jobs,int *numOfJobs){
    fprintf(stderr, "Error in system call\n");
    free(command);
    free(copy);
    free(args);
    freeJobs(jobs,numOfJobs);
    exit(1);
}

/**
 * This function runs over the running jobs and free them.
 * @param jobs - array of pointers to jobs
 * @param numOfJobs - the number of jobs in the array
 */
void freeJobs(struct job** jobs,int *numOfJobs){
    // free all the jobs and kill the processes
    int counter = 0;
    while (*numOfJobs){
        if (jobs[counter] != NULL){
            kill(jobs[counter]->pid, SIGKILL);
            free(jobs[counter]->commandArgs); // free the command
            free(jobs[counter]);
            (*numOfJobs)--;
        }
        counter++;
    }
    free(jobs);
}