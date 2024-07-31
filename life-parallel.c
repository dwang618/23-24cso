#define _XOPEN_SOURCE 600
#include "life.h"
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    LifeBoard *current;
    LifeBoard *next;
    pthread_barrier_t *sync;
    int id;
    int total;
    int iterations;
} ThreadArgs;

void thread_action(ThreadArgs *args) {
    /* division of work between threads, set proportion between number of threads and available columns*/
    /* excluding first and last column for race conditions and bc initializated at 0*/
    int open_cols = (args->current->width - 2);
    int first_col = (args->id * open_cols) / args->total;
    int last_col = ((args->id + 1) * open_cols) / args->total;

    /* duplicated logic from the serial_life implementation*/
    for (int step = 0; step < args->iterations; step += 1) {

        /* We use the range [1, width - 1) here instead of
         * [0, width) because we fix the edges to be all 0s.
         */
        for (int y = 1; y < args->current->height - 1; y += 1) {
            for (int x = first_col+1; x < last_col+1; x += 1) {
                
                /* For each cell, examine a 3x3 "window" of cells around it,
                 * and count the number of live (true) cells in the window. */
                int live_in_window = 0;
                for (int y_offset = -1; y_offset <= 1; y_offset += 1)
                    for (int x_offset = -1; x_offset <= 1; x_offset += 1)
                        if (LB_get(args->current, x + x_offset, y + y_offset))
                            live_in_window += 1;
                
                /* Cells with 3 live neighbors remain or become live.
                   Live cells with 2 live neighbors remain live. */
                LB_set(args->next, x, y,
                    live_in_window == 3 /* dead cell with 3 neighbors or live cell with 2 */ ||
                    (live_in_window == 4 && LB_get(args->current, x, y)) /* live cell with 3 neighbors */
                );
            }
        }
        
        /* wait before and after swap phase to ensure threads are in line, s.t. barrier allows for time to validate thread transition between boards*/
        pthread_barrier_wait(args->sync);
        /* now that we computed next_state, make it the current state */
        if(!(args->id)) {
            LB_swap(args->current, args->next);
        }
        pthread_barrier_wait(args->sync);

    }
    if(!(args->id)) {
        LB_del(args->next);
    }

    pthread_exit(NULL);
}

void simulate_life_parallel(int threads, LifeBoard *state, int steps) {
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, threads);

    LifeBoard *next_board = LB_new(state->width, state->height);          //setup for next state; taken from serial implementation
    pthread_t thread_arr[threads];                                        //thread array for reference 
    ThreadArgs thread_args[threads];                               

    /* iterate through the thread array, using pthread_create to establish 'int threads' of parallel threads*/
    for (int thread_id = 0; thread_id < threads; thread_id++) {
        thread_args[thread_id] = (ThreadArgs){ .id = thread_id, .total = threads, .iterations = steps, .current = state, .next = next_board, .sync = &barrier };  
        pthread_create(&thread_arr[thread_id], NULL, thread_action, &thread_args[thread_id]);
    }

    int num = 0;
    while(num<threads) {
        pthread_join(thread_arr[num], NULL);
        num++;
    }
    pthread_barrier_destroy(&barrier);
}
