#include <stdio.h> // fopen, fread, fclose, printf, fseek, ftell
#include <math.h> // log, exp
#include <stdlib.h> // free, realloc
#include <time.h> // struct timespec, clock_gettime, CLOCK_REALTIME
#include <errno.h>


// computes the geometric mean of a set of values.
// You should use OpenMP to make faster versions of this.
// Keep the underlying sum-of-logs approach.
double geomean(unsigned char *s, size_t n) {
    //ORIGINAL
    /*double answer = 0;
    for(int i=0; i<n; i+=1) {
        if (s[i] > 0) answer += log(s[i]) / n;
    }
    return exp(answer); */

    //EVEN SPLIT
    /*double answer = 0;
    #pragma omp parallel for 
    for(int i=0; i<n; i+=1) {
        if (s[i] > 0) {
            #pragma omp atomic update
            answer += log(s[i]) / n;
        }
    }
    return exp(answer); */

    //TASK QUEUE 
    /*double answer = 0;
    int j =0;
    #pragma omp parallel
    while (1) {
        int i;
        #pragma omp atomic capture
        i = j++;
        if (i >= n) break;
        // work on item i
        if (s[i] > 0) {
            #pragma omp atomic update
            answer += log(s[i]) / n;
        }

    }
    return exp(answer);  */

    //PARALLEL SCHEDULES
    /*double answer = 0;
    #pragma omp parallel for schedule(dynamic)
    for(int i=0; i<n; i+=1) {
        if (s[i] > 0) {
            #pragma omp atomic update
            answer += log(s[i]) / n;
        }
    }
    return exp(answer); */

    //MANY TO FEW REDUC
    /* double answer = 0;
    #pragma omp parallel
    {
        double localAnswer = 0;
        #pragma omp for nowait
        for(int i=0; i<n; i+=1) {
            if (s[i] > 0) {
                localAnswer += log(s[i]) / n;
            }
        }

        # pragma omp atomic update
        answer += localAnswer;
    }
    return exp(answer); */
}

/// nanoseconds that have elapsed since 1970-01-01 00:00:00 UTC
long long nsecs() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec*1000000000 + t.tv_nsec;
}


/// reads arguments and invokes geomean; should not require editing
int main(int argc, char *argv[]) {
    // step 1: get the input array (the bytes in this file)
    char *s = NULL;
    size_t n = 0;
    for(int i=1; i<argc; i+=1) {
        // add argument i's file contents (or string value) to s
        FILE *f = fopen(argv[i], "rb");
        if (f) { // was a file; read it
            fseek(f, 0, SEEK_END); // go to end of file
            size_t size = ftell(f); // find out how many bytes in that was
            fseek(f, 0, SEEK_SET); // go back to beginning
            s = realloc(s, n+size); // make room
            fread(s+n, 1, size, f); // append this file on end of others
            fclose(f);
            n += size; // not new size
        } else { // not a file; treat as a string
            errno = 0; // clear the read error
        }
    }

    // step 2: invoke and time the geometric mean function
    long long t0 = nsecs();
    double answer = geomean((unsigned char*) s,n);
    long long t1 = nsecs();

    free(s);

    // step 3: report result
    printf("%lld ns to process %zd characters: %g\n", t1-t0, n, answer);
}
