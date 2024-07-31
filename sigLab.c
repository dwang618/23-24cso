#define _XOPEN_SOURCE 700 // request all POSIX features, even with -std=c11
#include <stdlib.h>	// For EXIT_SUCCESS, NULL, abort
#include <stdio.h>	// for getline
#include <time.h>	// for nanosleep
#include <unistd.h>	// for getpid
#include <signal.h>	// signal, kill, sigint, sigterm, sigusr1
#include <string.h>	// strcpy, strlen
#include <sys/mman.h>	// shm_open, shm_unlink, mmap, prot_read, prot_write
#include <fcntl.h>	// 0_creat, 0_rdwr
#include <sys/stat.h>	// mode constants

pid_t other_pid = 0;
char *inbox = NULL;
char *outbox = NULL;

// destroying inbox / outbox    
void cleanup() {
	printf("cleaning");
	if (inbox != NULL) {
		free(inbox);
		inbox = NULL;
	}
	if (outbox != NULL) {
		shm_unlink(outbox);
		outbox = NULL;
	}		
}

// SIGINT and SIGTERM direct here
void sig_handler(int signum) {
	if (signum == SIGINT || signum == SIGTERM) {
		//cleanup();
        printf("i am the sender");
		if(other_pid != 0) {
			kill(other_pid, SIGTERM); // sigterm to other user
		}
		exit(EXIT_SUCCESS);
	} else if (signum == SIGUSR1) {
        printf("i am the reciever");
		if (inbox != NULL) {
			//printf("her	e");
			//printf("Inbox contents: %s\n", inbox);
			//fputs("Inbox contents: %s\n", inbox);
			///strcpy(inbox, "lezzz gooooo");  // Copy the new string to inbox
			puts(outbox);
			fflush(stdout);	
			inbox[0] = '\0';
		} else {
			printf("Empty inbox. \n");
		}
	} 

}

int main(void) {
	struct sigaction sa;
	sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;


	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	
	printf("This process's ID: %ld\n", (long) getpid());
	
	// unque shared memory region based on PID
	char inbox_name[512];
	char outbox_name[512];
	snprintf(inbox_name, sizeof inbox_name, "/%d-inbox", getpid());
	snprintf(outbox_name, sizeof outbox_name, "/%d-outbox", other_pid());

	// Create and initialize shared memory regions for inbox and outbox
	int inbox_fd = shm_open(inbox_name, O_CREAT | O_RDWR, 0666);
	if (inbox_fd == -1) {
		perror("shm_open");
		cleanup();
		exit(EXIT_FAILURE);
	}
	if (ftruncate(inbox_fd, 4096) == -1) {
		perror("ftruncate");
		cleanup();
		exit(EXIT_FAILURE);
	}

	inbox = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, inbox_fd, 0);
	//printf(inbox);
	//printf(" is what inbox would look like if it had some shit in ther ebro");
	if(inbox == MAP_FAILED) {
		perror("mmap");
		cleanup();
		exit(EXIT_FAILURE);
	}
	close(inbox_fd);

	int outbox_fd = shm_open(outbox_name, O_CREAT | O_RDWR, 0666);
	if(outbox_fd == -1) {
		perror("shm_open");
		cleanup();
		exit(EXIT_FAILURE);
	}
	if(ftruncate(outbox_fd, 4096) == -1) {
		perror("ftruncate");
		cleanup();
		exit(EXIT_FAILURE);
	}
	outbox = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, outbox_fd, 0);
	if(outbox == MAP_FAILED) {
		perror("mmap");
		cleanup();
		exit(EXIT_FAILURE);
	}
	close(outbox_fd);

	char *line = NULL; size_t line_length = 0;
	do {
		printf("Enter other process ID: ");
		if(-1 == getline(&line, &line_length, stdin)) {
			perror("getline");
			abort();
		}
	} while ((other_pid = strtol(line, NULL, 10)) == 10);
	free(line);
/*
	// non-starter code starts below
	inbox = (char *)malloc(256 * sizeof(char));
	strcpy(inbox, "bruh"); // temp populating
	// keep programming running
	while(1) {
		sleep(1);
	}
*/

	while (1) {
        printf("type message: ");
        char *line = NULL;
        size_t line_length = 0;

        if (-1 == getline(&line, &line_length, stdin)) {
            perror("getline");
            cleanup();
            exit(EXIT_FAILURE);
        }

        // Wait for the receiver to acknowledge readiness
        while (inbox[0] != '\0') {
            usleep(10000);  // Sleep for 10 milliseconds
        }

        // Copy the message to the outbox
        strncpy(outbox, line, 512);

		/*puts(outbox);
		fflush(stdout);*/

    // Notify the other process using SIGUSR1
        kill(other_pid, SIGUSR1);

        free(line);
    }


	cleanup();
	return EXIT_SUCCESS;
}