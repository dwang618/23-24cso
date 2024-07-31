#define _XOPEN_SOURCE 700
#include <stdlib.h> 
#include <stdio.h>	
#include <time.h>	
#include <sys/stat.h>
#include <unistd.h>	
#include <signal.h>	
#include <fcntl.h>	

//other pid reference to alternate process in case -1
pid_t other_pid;
volatile sig_atomic_t global_flag = 0;
int numReps = 1000;


/// returns the number of nanoseconds that have elapsed since an arbitrary time
long long nsecs() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}

void sig_handler(int signum) {
    global_flag = 1;
}



void emptyFunctionCall() {

}

long long calcAverageTime(long long retTime, int reps) {
    return retTime / (long) reps;
}

void scenario1(FILE *timingsFile) {
    long long retTime = 0;
    int loopIteration = 1;
    numReps = 350000;
    for(int i = 0; i < numReps; i++) {
        
        long long startTimer = nsecs();
        emptyFunctionCall();
        long long endTimer = nsecs();

        retTime += endTimer - startTimer;

        if (loopIteration % (numReps/10)== 0) {
            fprintf(timingsFile, "Scenario 1 @ Loop Iteration %d: Runtime = %lld ns \n", loopIteration, retTime);
        }
        loopIteration++;
        
    }
    fprintf(timingsFile, "Scenario 1 Average Time: %lld nanoseconds\n\n", calcAverageTime(retTime, numReps));
    fclose(timingsFile);
}

void scenario2(FILE *timingsFile) {
    long long retTime = 0;
    int loopIteration = 1;
    numReps = 70000;

    for(int i = 0; i < numReps; i++) {
        
        long long startTimer = nsecs();
        getppid();
        long long endTimer = nsecs();

        retTime += endTimer - startTimer;

        if (loopIteration % (numReps/10)== 0) {
            fprintf(timingsFile, "Scenario 2 @ Loop Iteration %d: Runtime = %lld ns \n", loopIteration, retTime);
        }
        loopIteration++;
    }
    fprintf(timingsFile, "Scenario 2 Average Time: %lld nanoseconds\n\n", calcAverageTime(retTime, numReps));
    fclose(timingsFile);
}

void scenario3(FILE *timingsFile) {
    long long retTime = 0;
    int loopIteration = 1;
    numReps = 100;

    for(int i = 0; i < numReps; i++) {
        
        long long startTimer = nsecs();
        system("/bin/true");
        long long endTimer = nsecs();

        retTime += endTimer - startTimer;

        if (loopIteration % (numReps/10)== 0) {
            fprintf(timingsFile, "Scenario 3 @ Loop Iteration %d: Runtime = %lld ns \n", loopIteration, retTime);
        }
        loopIteration++;
    }
    fprintf(timingsFile, "Scenario 3 Average Time: %lld nanoseconds\n\n", calcAverageTime(retTime, numReps));
    fclose(timingsFile);
}

void scenario4(FILE *timingsFile) {
    int loopIteration = 1;
    numReps = 5000;
    struct timespec sleepDuration, remainingTime;
    //set the sleep duration
    sleepDuration.tv_sec = 1;                     // seconds
    sleepDuration.tv_nsec = 500000000/numReps;    // nanoseconds (0.5 seconds) relative to iterations

    long long retTime = 0;

    for(int i = 0; i < numReps; i++) {
        
        long long startTimer = nsecs();
        raise(SIGUSR1);
        while(global_flag != 1) {
            nanosleep(&sleepDuration, &remainingTime);
        }
        long long endTimer = nsecs();

        retTime += endTimer - startTimer;

        if (loopIteration % (numReps/10)== 0) {
            fprintf(timingsFile, "Scenario 4 @ Loop Iteration %d: Runtime = %lld ns \n", loopIteration, retTime);
        }
        loopIteration++;
    }
    global_flag =0;
    fprintf(timingsFile, "Scenario 4 Average Time: %lld nanoseconds\n\n", calcAverageTime(retTime, numReps));
    fclose(timingsFile);
}

void scenario5(FILE *timingsFile, sigset_t waitMask) {
    int loopIteration = 1;
    long long retTime = 0;
    numReps = 10000;

    printf("The current process ID is: %ld\n", (long) (getpid()));

    //taken from signals lab
    char *line = NULL; 
    size_t line_length = 0;
	do {
        printf("Enter other process ID: ");
        if (-1 == getline(&line, &line_length, stdin)) {
            perror("getline");
            abort();
        }
        other_pid = strtol(line, NULL, 10);
        fflush(stdin);  // Clear input buffer

    } while (other_pid == 10 || other_pid <= 0);


    //printf("im working");
	free(line);


    for (int i = 0; i < numReps; i++) {
        //printf("meowmoew");
        long long startTimer = nsecs();
        kill(other_pid, SIGUSR1);
        sigwait(&waitMask, &global_flag);
        long long endTimer = nsecs();
        retTime += endTimer - startTimer;

        if (loopIteration % (numReps/10)== 0) {
            fprintf(timingsFile, "Scenario 5 @ Loop Iteration %d: Runtime = %lld ns\n", loopIteration, retTime);
        }
        loopIteration++;
        global_flag = 0;
    }
    fprintf(timingsFile, "Scenario 5 Average Time: %lld nanoseconds\n\n", calcAverageTime(retTime, numReps));
    fclose(timingsFile);
}

void scenarioMinus1(FILE *timingsFile, sigset_t waitMask) {
    numReps = 10000;
    long long retTime = 0;
    printf("The current process ID is: %ld\n", (long) (getpid()));

    //taken from signals lab
    char *line = NULL; 
    size_t line_length = 0;
	do {
    printf("Enter other process ID: ");
        if (-1 == getline(&line, &line_length, stdin)) {
            perror("getline");
            abort();
        }
        other_pid = strtol(line, NULL, 10);
        fflush(stdin);  //clear input buffer

    } while (other_pid == 10 || other_pid <= 0);

    free(line);

    for (int i = 0; i < numReps; i++) {
        sigwait(&waitMask, &global_flag);
        kill(other_pid, SIGUSR1);
    }

}



int main(int argc, char *argv[]) {
    //open timings text file with append parameter to not overwrite previous text
    FILE *timingsFile = fopen("timings.txt", "a");

    //define setup for signal handler, register signal under SIGUSR1 
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    //define setup for signal mask to block specific signals in sigset
    //call to sigwait will pause execution until sigusr1 recieved
    sigset_t waitMask;
    sigemptyset(&waitMask);
    sigaddset(&waitMask, SIGUSR1);
    
    //second argument in array denoting which case number/function to run
    int scenarioNumber = atoi(argv[1]);
    if(scenarioNumber == -1) {
        scenarioMinus1(timingsFile, waitMask);
    }
    else if(scenarioNumber == 1) {
        scenario1(timingsFile);
    }
    else if(scenarioNumber == 2) {
        scenario2(timingsFile);
    }
    else if(scenarioNumber == 3) {
        scenario3(timingsFile);
    }
    else if(scenarioNumber == 4) {
        scenario4(timingsFile);
    }
    else if(scenarioNumber == 5) {
        scenario5(timingsFile, waitMask);
    }
    else {
        printf("you broke it bruh u dumb as sht lmao");
        return -1;
    }
    return 0;
}


