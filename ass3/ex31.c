// Lee alima 313467441

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define END_OF_FILE 0
#define LENGTH_ERROR_MSG 21
#define ERROR_IN_SC -1
#define FAIL 4
#define STDERR 2
#define DIFFERENT 1
#define SIMILAR 2
#define EQUAL 3

int createFiles(char **args);
int readFiles(int firstFile, int secFile);
int checkIfSimilar(int firstFile, int secFile,
                   char firstBufferFile, char secBufferFile);
int readOnlyOneFile(int file,char buffer,int otherFile);
void closeAndExit(int firstFile, int secFile);

/**
 * This is the main function of the program,
 * In this function I called createFiles function which runs the all program.
 * @param argc - number of arguments
 * @param argv - expected for two files with their paths.
 * @return 3 if both files are equals, 2 if both file are similar
 * 1 if they are not equal and not similar.
 * The program exits with 4 if there were an error in a system call
 */
int main(int argc, char **argv) {
    return createFiles(argv);
}

/**
 * This function opens the files based on the arguments
 * that were given in main, than calls readFiles with those files
 * @param args - expected for two files with their paths.
 * @return what readFiles returns
 */
int createFiles(char **args){
    //FILE *firstFile, *secFile;
    int firstFile,secFile,returnVal;
    // try to open the first file
    firstFile = open(args[1],O_RDONLY);
    if (firstFile == ERROR_IN_SC){ // error in opening the first file
        write(STDERR,"Error in system call\n",LENGTH_ERROR_MSG);
        exit(FAIL);
    }
    // try to open the second file
    secFile = open(args[2],O_RDONLY);
    if (secFile == ERROR_IN_SC){ // error in opening the second file
        close(firstFile);
        write(STDERR,"Error in system call\n",LENGTH_ERROR_MSG);
        exit(FAIL);
    }
    returnVal = readFiles(firstFile,secFile);
    // close files
    close(firstFile);
    close(secFile);
    return returnVal;
}

/**
 * This function checks if both files are equal by reading char by char from
 * both files, if the loop has ended it means that the files are equal.
 * In case than the chars are not equal I called checkIfSimilar to check
 * if the files are just similar.
 * @param firstFile - the first file (created in CreatesFiles)
 * @param secFile - the second file (created in CreatesFiles)
 * @return  3 if both files are equals, 2 if both file are similar
 * 1 if they are not equal and not similar.
 * The program exits with 4 if there were an error in a system call
 */
int readFiles(int firstFile, int secFile){
    unsigned char bufferFile1, bufferFile2;
    int flag1,flag2;
    // read the first char in every file
    flag1 = read(firstFile,&bufferFile1,sizeof(char));
    flag2 = read(secFile,&bufferFile2,sizeof(char));
    // check reading
    if (flag1 == ERROR_IN_SC || flag2 == ERROR_IN_SC){ // error
        closeAndExit(firstFile,secFile);
    }
    // while both files haven't gotten to end continue
    while (flag1 != END_OF_FILE && flag2 != END_OF_FILE){
        // if the two chars are equal, than continue reading
        if (bufferFile1 == bufferFile2){
            flag1 = read(firstFile,&bufferFile1,sizeof(char));
            flag2 = read(secFile,&bufferFile2,sizeof(char));
            if (flag1 == ERROR_IN_SC || flag2 == ERROR_IN_SC){ // error
                closeAndExit(firstFile,secFile);
            }
        } else { // if the two chars are not equal, than check if the are
            // "similar" and return what checkIfSimilar returns
            return checkIfSimilar(firstFile,secFile,bufferFile1,bufferFile2);
        }
    }
    // if both files ended and there was not calling to checkIfSimilar
    if (flag1 == END_OF_FILE && flag2 == END_OF_FILE){ // identical
        return EQUAL;
    } else if (flag1 != END_OF_FILE && flag2 == END_OF_FILE){
        // if reading the first file hasn't finished but reading the second
        // file did finish
        return readOnlyOneFile(firstFile,bufferFile1,secFile);
    } else if (flag2 != END_OF_FILE && flag1 == END_OF_FILE){
        // if reading the second file hasn't finished but reading the first
        // file did finish
        return readOnlyOneFile(secFile,bufferFile2,firstFile);
    }
}

/**
 * This function is called from readFiles in case of different chars.
 * In this function I skipped every kind of space, and checked if the chars
 * are similar or equal. If they didn't I return 1, otherwise, I continue
 * reading chars from the files
 * @param firstFile - the first file
 * @param secFile  - the second file
 * @param firstBufferFile - the last char that was read from file 1
 * @param secBufferFile - the last char that was read from file 2
 * @return 2 if the files are similar and 1 if not.
 */
int checkIfSimilar(int firstFile, int secFile,
                   char firstBufferFile, char secBufferFile){
    int flag1 = 1,flag2=1,flag = 0;
    char bufferFile1 = firstBufferFile;
    char bufferFile2 = secBufferFile;
    // while reading both files havn't finished
    while (flag1 != END_OF_FILE && flag2 != END_OF_FILE){
        flag = 0;
        // while reading a kind of space -> read the next char
        while(bufferFile1 == ' ' || bufferFile1 == '\n'
               || bufferFile1 == '\t'){
            flag1 = read(firstFile,&bufferFile1,sizeof(char));
            if (flag1 == ERROR_IN_SC){ // error
                closeAndExit(firstFile,secFile);
            }
            if (flag1 == END_OF_FILE){ // reading first file has ended
                break;
            }
        }
        // while reading a kind of space -> read the next char
        while(bufferFile2 == ' ' || bufferFile2 == '\n'
               || bufferFile2 == '\t'){
            flag2 = read(secFile,&bufferFile2,sizeof(char));
            if (flag2 == ERROR_IN_SC){ // error
                closeAndExit(firstFile,secFile);
            }
            if (flag2 == END_OF_FILE){  // reading second file has ended
                break;
            }
        }
        // if buffers are equal
        if (bufferFile1 == bufferFile2){
            flag = 1;
        } else if (bufferFile1 >= 'A'&& bufferFile1 <= 'Z'
                    && bufferFile2 >= 'a' && bufferFile2 <= 'z'){
            // check big and small letters
            if (bufferFile1 + 32 == bufferFile2){
                flag = 1;
            } else {
                return DIFFERENT;
            }
        } else if (bufferFile1 >= 'a'&& bufferFile1 <= 'z'
                   && bufferFile2 >= 'A' && bufferFile2 <= 'Z'){
            // check big and small letters
            if (bufferFile1-32 == bufferFile2){
                flag = 1;
            } else {
                return DIFFERENT;
            }
        } else { // if letters are not equal and not similar
            return DIFFERENT;
        }
        if (flag){ // if letters are equal or similar continue reading files
            flag1 = read(firstFile,&bufferFile1,sizeof(char));
            flag2 = read(secFile,&bufferFile2,sizeof(char));
            if (flag1 == ERROR_IN_SC || flag2 == ERROR_IN_SC){ // error
                closeAndExit(firstFile,secFile);
            }
        }
    }
    // if both files ended it means they are similar
    if (flag1 == END_OF_FILE && flag2 == END_OF_FILE){
        return SIMILAR;
    } else if (flag2 != END_OF_FILE && flag1 == END_OF_FILE){
        // if reading second file hasn't finished
        return readOnlyOneFile(secFile,bufferFile2,firstFile);
    }
    else if (flag1 != END_OF_FILE && flag2 == END_OF_FILE){
        // if reading first file hasn't finished
        return readOnlyOneFile(firstFile,bufferFile1,secFile);
    }
}

/**
 * This function is called whenever reading one file has ended but
 * reading the other hasn't
 * @param file - The file that reading it hasn't finished
 * @param buffer - the last char that was read from the file
 * @return 2 -if the rest of the file contains only kind of spaces, else 1.
 */
int readOnlyOneFile(int file,char buffer,int otherFile){
    int flag = 1;
    char bufferFile = buffer;
    while (flag != END_OF_FILE && (bufferFile == ' ' || bufferFile == '\n'
                                   || bufferFile == '\t')){
        flag = read(file,&bufferFile,sizeof(char));
        if (flag == ERROR_IN_SC){ // error
            closeAndExit(file,otherFile);
        }
    }
    if (flag == END_OF_FILE){
        return SIMILAR;
    } else { // read a char that is not a kind of space
        return DIFFERENT;
    }
}

/**
 * This function is called in case of error in system call,
 * It closed the files and exit
 * @param firstFile - the first file (fd)
 * @param secFile - the second file (fd)
 */
void closeAndExit(int firstFile, int secFile) {
    close(firstFile);
    close(secFile);
    write(STDERR,"Error in system call\n",LENGTH_ERROR_MSG);
    exit(FAIL);
}