#define _XOPEN_SOURCE 700
#include <unistd.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Definition of O_* constants */
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>



char *getoutput(const char *command) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    char* output = malloc(4096);
    pid_t pid;
    pid = fork();

    if (pid == -1) {     
        perror("terminate attempthere fork failure");  
        return NULL;                                                 // check for error
    }
    if (pid == 0) {     
        if (dup2(pipe_fd[1], 1) == -1) {                //call dup2, linking write end of pipe to stdout 
            perror("dup2 faiure, does not allocate a new file descriptor as copy of old fd");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[0]);                                          //close both pipe file descriptors
        close(pipe_fd[1]);

        execl("/bin/sh", "sh", "-c", command, (char*)NULL);         //call execl to setup a new process via terminal arguments

        exit(1);
    } else {
        close(pipe_fd[1]);

        FILE *readPipe = fdopen(pipe_fd[0], "r");                   //call fdopen to wrap fd to file
        if (readPipe == NULL) {
            perror("fdopen failure, unable to wrap the file descriptor of read end of pipe as a FILE");
            exit(EXIT_FAILURE);
        }

        size_t len = 1;
        //ssize_t inputBuffer = 1;
        ssize_t input;

        char *stream = malloc(1);


        //printf("%s\n", "i get here 2");  
        stream[0] = '\0';      
        while((input = getdelim(&output, &len, '\0', readPipe)) != -1) {
            stream = realloc(stream, input + len);
            len += input;
            strcat(stream, output);
        }     //use getdelim to allocate memory and read the stream from a file until delimiter

        close(pipe_fd[0]);
        
        if (waitpid(pid, NULL, 0) == -1) {
		    perror("waitpid");
            exit(EXIT_FAILURE);
        }

        return stream;
    }
}

char *parallelgetoutput(int count, const char **argv_base) {
    pid_t pid;

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    char* output = malloc(4096);
    char *stream = malloc(1);
    stream[0] = '\0';

    for (int x = 0; x < count; x++) {
        pid = fork();
        if (pid == -1) {     
            perror("terminate attempthere fork failure");  
            return NULL;                                                 // check for error
        }

        if (pid == 0) {

            dup2(pipe_fd[1], 1);

            close(pipe_fd[0]);                                          //close both pipe file descriptors
            close(pipe_fd[1]);

            char buffer[sizeof(int)+1];
            snprintf(buffer, sizeof(buffer), "%d", x);

            int numArgs = array_length(argv_base);
            int outputArgs = numArgs +2;
            char **argv_childProcess = malloc(sizeof(char *) * outputArgs); //allocate for size of pointer array
            if (argv_childProcess == NULL) {
                perror("malloc error womp womp");
                exit(EXIT_FAILURE);
            }   

            int ind = 0;
            while(ind < outputArgs) {
                if(ind < numArgs) {
                    argv_childProcess[ind] = argv_base[ind];
                }
                else if(ind == numArgs){
                    argv_childProcess[ind] = buffer;
                }
                else {
                    argv_childProcess[ind] = NULL;
                }
                ind++;
            }
        

            execv(argv_base[0], (char *const *)argv_childProcess);
            perror("execv");
            exit(EXIT_FAILURE);
        }
    }
    close(pipe_fd[1]);

    FILE *readPipe = fdopen(pipe_fd[0], "r");                   //call fdopen to wrap fd to file
    if (readPipe == NULL) {
        perror("fdopen failure, unable to wrap the file descriptor of read end of pipe as a FILE");
        exit(EXIT_FAILURE);
    }

    size_t len = 1;
    //ssize_t inputBuffer = 1;
    ssize_t input;

    //printf("%s\n", "i get here 2");  
    stream[0] = '\0';      
    while((input = getdelim(&output, &len, '\0', readPipe)) != -1) {
        stream = realloc(stream, input + len);
        len += input;
        strcat(stream, output);
    }     //use getdelim to allocate memory and read the stream from a file until delimiter

    close(pipe_fd[0]);
    for (int i = 0; i < count; i++) {
        wait(NULL);
    }

    return stream;
}

int array_length(const char **array) {
    int length = 0;

    while (array[length] != NULL) {
        length++;
    }

    return length;
}
