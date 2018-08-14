// Lee Alima 313467441

#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <wait.h>
#include <stdio.h>
#include <time.h>

#define STDERR 2
#define LENGTH_ERROR_MSG 21
#define FAIL -1
#define BUFFER_SIZE 160
#define MSG_LENGTH 20
#define GRADE_LENGTH 10

#define NO_C_FILE "0"
#define COMPILATION_ERROR "0"
#define TIMEOUT "0"
#define BAD_OUTPUT "60"
#define SIMILAR_OUTPUT "80"
#define GREAT_JOB "100"

#define NO_C_FILE_MSG "NO_C_FILE"
#define COMPILATION_ERROR_MSG "COMPILATION_ERROR"
#define TIMEOUT_MSG "TIMEOUT"
#define BAD_OUTPUT_MSG "BAD_OUTPUT"
#define SIMILAR_OUTPUT_MSG "SIMILAR_OUTPUT"
#define GREAT_JOB_MSG "GREAT_JOB"

#define SAVE_FILE_FOR_COMP "Lee.txt"
#define RESULT_FILE "results.csv"

/**
 * I save the information about every student directory
 * in a unique struct.
 */
struct Student {
    char DirectoryName[BUFFER_SIZE]; //relative path
    char CFilePath[BUFFER_SIZE]; // "" if c file doesn't exist
    char grade[GRADE_LENGTH];
    char msg[MSG_LENGTH];
};

void handleMission(char **argv);
void readFile(char* path,char matrix[3][BUFFER_SIZE]);
void searchInDir(char path[BUFFER_SIZE], char* fillStruct);
int checkIfCFile(char name[BUFFER_SIZE]);
void complieCFile(struct Student *student,char input[BUFFER_SIZE], char output[BUFFER_SIZE]);
void workWithCompileFile(struct Student *student,char input[BUFFER_SIZE], char output[BUFFER_SIZE]);
void compareFILES(struct Student *student,char first[BUFFER_SIZE], char sec[BUFFER_SIZE]);
void handleResultFile(struct Student *students,int index);
void handleFailure();
int calculateTime(pid_t pid);

/**
 * This is the main function which runs the project
 * @param argc - number of arguments ( should be only 2)
 * @param argv - name and path of the config file
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2){
        handleFailure();
    }
    handleMission(argv);
    return 0; // program ended successfully
}

/**
 * This function reads the file, creates struct "Student" for each
 * directory in the "Students" dir and fill it with the right parameters.
 * at the end I called a function to write the students info to the result file
 * @param argv - name and path of config file
 */
void handleMission(char **argv){
    DIR *dir;
    struct dirent *entry;
    int allocated, index =0;
    struct Student *backup;
    // create buffer to save the info from the file
    char matrix[3][BUFFER_SIZE];
    readFile(argv[1], matrix); // read the file
    // save data from the file in the buffers
    char firstLine[BUFFER_SIZE] ="";
    strcpy(firstLine,matrix[0]);
    char secLine[BUFFER_SIZE] ="";
    strcpy(secLine,matrix[1]);
    char thirdLine[BUFFER_SIZE] ="";
    strcpy(thirdLine,matrix[2]);
    if (strcmp(firstLine,"")==0 || strcmp(secLine,"")==0||strcmp(thirdLine,"") == 0){
        // for empty line in the config file
        // I wrote an error in a system call even thought the error is not
        // in a system call because its the pattern of msg asked in the Targil
        handleFailure();
    }
    // create the first structs
    struct Student *students = calloc(4,sizeof(struct Student));
    if (students == NULL){
        handleFailure();
    }
    allocated = 4;
    if ((dir = opendir(firstLine)) ==  NULL){
        free(students);
        handleFailure();
    }
    while ((entry = readdir(dir)) != NULL) { // go over the directories
        if (index == allocated){ // check if buffer is full
            allocated *= 2; // allocate more place
            backup = realloc(students,allocated*sizeof(struct Student));
            if (backup ==  NULL){ // allocation failed
                free(students);
                handleFailure();
            }
            students = backup;
        }
        char fullPath[BUFFER_SIZE] =""; // for specific student directory
        snprintf(fullPath, sizeof(fullPath), "%s/%s", firstLine, entry->d_name);
        if (entry->d_type == DT_DIR) { // check if the type is a directory
            if (strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0){
                strcpy(students[index].DirectoryName,entry->d_name); // dir name
                strcpy(students[index].CFilePath,"");
                searchInDir(fullPath,students[index].CFilePath);
                index++;
            }
        }
    } // try to complie and run the files
    int t;
    for (t = 0; t<index;t++){
        complieCFile(&students[t],secLine,thirdLine);
    }
    handleResultFile(students,index); // write info to result file
    free(students);
    if (closedir(dir) == FAIL){
        handleFailure();
    }
}

/**
 * This function is responsible of reading the config file
 * @param path - the path of the config file
 * @param matrix - buffer to save the lines from the file in
 */
void readFile(char *path, char matrix[3][BUFFER_SIZE]){
    int index=0; // index in the matrix (for each line)
    int num = 0; // num of line
    int cont, i;
    int fd = open(path,O_RDONLY); // try to open the file
    if (fd < 0){ // error in opening the first file
        handleFailure();
    }
    char buffer[BUFFER_SIZE];
    while((cont = read(fd,buffer,sizeof(buffer)))>0){
        for (i = 0; i<cont;i++){
            if (buffer[i] != '\n' && buffer != NULL){
                matrix[num][index] = buffer[i];
                index++;
            } else { // move to the next line
                matrix[num][index]= '\0';
                num++;
                index=0;
            }
        }
    }
    if (cont < 0){
        handleFailure();
    }
}

/**
 * This function is responsible of finding c files in directories
 * This is a recursive function
 * @param name - name of the path
 * @param fillStruct - student struct to fill with c file path
 */
void searchInDir(char name[BUFFER_SIZE],char* fillStruct){
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(name)) ==  NULL){ // check if fail
        handleFailure();
    }
    while ((entry = readdir(dir)) != NULL) { // more objects in dir
        if (entry->d_type == DT_DIR) { // if directory found
            char path[BUFFER_SIZE];
            // ignore . and ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            // save in name the new path to search the c file
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            searchInDir(path,fillStruct);
        } else {
            if (checkIfCFile(entry->d_name)){
                char localBuffer[BUFFER_SIZE] ="";
                // save the path for the c file in localBuffer
                snprintf(localBuffer, sizeof(localBuffer), "%s/%s", name, entry->d_name);
                strcpy(fillStruct,localBuffer);
                if (closedir(dir) == FAIL) { // close dir in case that c file was found
                    handleFailure();
                }
                return;
            }
        }
    }
    if (closedir(dir) == FAIL) { // close dir in case that no c file was found
        handleFailure();
    }
}

/**
 * This function checks if a file is a c file by checking
 * the last to chars in its name
 * @param name - the file name
 * @return 1 if the ending of the file is ".c" ,otherwise 0
 */
int checkIfCFile(char *name) {
    int size = strlen(name);
    if (name[size-2] == '.' && tolower(name[size-1]) == 'c'){
        return 1;
    }
    return 0;
}

/**
 * Try to compile the c file, if compilation done successfully than
 * work with compile file, otherwise update info in student
 * @param student - struct
 * @param input - input for running the exe file
 * @param output - the correct output
 */
void complieCFile(struct Student *student, char input[BUFFER_SIZE],
                  char output[BUFFER_SIZE]) {
    if (strcmp(student->CFilePath,"") == 0){
        strcpy(student->grade ,NO_C_FILE);
        strcpy(student->msg, NO_C_FILE_MSG);
    } else { // c file exists
        int status;
        pid_t p = fork();
        if (p == 0){ // son
                char* args[3] = {"gcc", student->CFilePath,NULL};
                execvp(args[0],args);
                handleFailure();
        } else if (p<0){ // error in fork
            handleFailure();
        } else {
            if (waitpid(p, &status, 0) == FAIL){ // wait for son
                handleFailure();
            }
            // check if compilation done
            if (WEXITSTATUS(status) == 1){ // error
                strcpy(student->grade,COMPILATION_ERROR);
                strcpy(student->msg, COMPILATION_ERROR_MSG);
                return;
            } else { // work with compiled file
                workWithCompileFile(student,input,output);
            }
        }
    }
}

/**
 * run the progress using dup for input and output, checking if
 * executing the exe takes more than 5 sec.
 * @param student - struct
 * @param input - input for running the exe file
 * @param output - the correct output
 */
void workWithCompileFile(struct Student *student, char *input, char *output) {
    int inputFD, outputFD;
    pid_t p = fork();
    if (p == 0){ // son
        if ((inputFD = open(input,O_RDONLY)) < 0){
            handleFailure();
        }
        if ((outputFD = open(SAVE_FILE_FOR_COMP,O_CREAT|O_TRUNC|O_WRONLY,0666)) < 0) {
            handleFailure();
        }
        if (dup2(inputFD,STDIN_FILENO) == FAIL) { // get input from inputFD
            handleFailure();
        }
        if (dup2(outputFD,STDOUT_FILENO) == FAIL){ // save output in outputFD
            handleFailure();
        }
        if (close(inputFD) == FAIL || close(outputFD)){
            handleFailure();
        }
        char* args[2] = {"./a.out", NULL};
        execvp(args[0],args);
        handleFailure();
    } else if ( p< 0){
        handleFailure();
    } else {
        int bool = calculateTime(p);
        if ((unlink("a.out") == FAIL)){
            handleFailure();
        }
        if (bool == 0){ // false means that the son hasn't finished
            strcpy(student->grade,TIMEOUT);
            strcpy(student->msg,TIMEOUT_MSG);
            if (unlink(SAVE_FILE_FOR_COMP) == FAIL){
                handleFailure();
            }
            return;
        } // compare files
        compareFILES(student,SAVE_FILE_FOR_COMP,output);
    }
}

/**
 * In this function I compares the output file (from running the prog) and
 * the correct path (given its pat in the config file) using the comp.out
 * from the ex3 part 1
 * @param student - struct
 * @param first - path to the first file
 * @param sec - path to the sec file
 */
void compareFILES(struct Student *student, char *first, char *sec) {
    int status;
    pid_t p = fork();
    if (p==0){ // son
        char* args[4]={"./comp.out",first,sec,NULL};
        execvp(args[0],args);
        handleFailure();
    } else if (p<0){
        handleFailure();
    } else {
        if (waitpid(p,&status,0) == FAIL){ // wait for son
            handleFailure();
        }
        switch (WEXITSTATUS(status)){ // the result from the last targil
            case 1:
                strcpy(student->grade,BAD_OUTPUT);
                strcpy(student->msg,BAD_OUTPUT_MSG);
                break;
            case 2:
                strcpy(student->grade,SIMILAR_OUTPUT);
                strcpy(student->msg, SIMILAR_OUTPUT_MSG);
                break;
            case 3:
                strcpy(student->grade,GREAT_JOB);
                strcpy(student->msg,GREAT_JOB_MSG);
                break;
        }
        if (unlink(SAVE_FILE_FOR_COMP) == FAIL){
            handleFailure();
        }
    }
}

/**
 * This function goes over the students and write the asked data
 * the the results.csv file
 * @param students -dynamic array of students
 * @param index - number of students
 */
void handleResultFile(struct Student *students,int index) {
    int outputFD = open(RESULT_FILE,O_CREAT|O_WRONLY,0666);
    if (outputFD < 0){
        handleFailure();
    }
    char buffer[BUFFER_SIZE] ="";
    int i;
    // go over all of the students and write to the file their data
    for (i = 0;i<index;i++){
        // save data in buffer
        snprintf(buffer, sizeof(buffer), "%s,%s,%s\n",
                 students[i].DirectoryName, students[i].grade,students[i].msg);
        if (write(outputFD,buffer,strlen(buffer)) == FAIL){
            handleFailure();
        }
        memset(buffer,0,BUFFER_SIZE);
    }
    if (close(outputFD)== FAIL){
        handleFailure();
    }
}

/**
 * This function checks if 5 sec have passed since the son
 * started to run until he done, calculating the target time
 * and compare the current time to it
 * @param pid - the son pid
 * @return 1 if he done, otherwise 0
 */
int calculateTime(pid_t pid){
    int timeAfter5Sec = time(NULL)+5;
    int status;
    while (time(NULL)<timeAfter5Sec){
        status =waitpid(pid,0,WNOHANG);
        if (status == FAIL){
            handleFailure();
        }
        if (status != 0 ){
            return 1;
        }
    }
    return 0;
}

/**
 * This function is being called in case of an error,
 * I wrote a msg for the user and exit the program
 */
void handleFailure(){
    write(STDERR,"Error in system call\n",LENGTH_ERROR_MSG);
    exit(FAIL);
}