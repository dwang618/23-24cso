#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int my_system(const char *command) {
    int status;
    pid_t pid;

    pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid == 0) {
        char *argv[4];
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = (char *)command;
        argv[3] = NULL;
        execvp(argv[0], argv);
        exit(1);
	//exit(WEXITSTATUS(status));
    } else {
        if (waitpid(pid, &status, 0) == -1) {
		return -1;
        } else if (WIFEXITED(status)) {
		//printf("Shell exit status: %d\n", WEXITSTATUS(status));
		return status; //WEXITSTATUS(status);
        }
        	return -1;
    }
}
