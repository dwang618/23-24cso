#define _GNU_SOURCE
#include "util.h"
#include <stdio.h>      // for printf
#include <stdlib.h>     // for atoi (and malloc() which you'll likely use)
#include <sys/mman.h>   // for mmap() which you'll likely use
#include <stdalign.h>
#include <string.h>
#include <errno.h>


alignas(4096) volatile char global_array[4096 * 32];

void labStuff(int which) {
    if (which == 0) {
        /* do nothing */
    } else if (which == 1) {
        char result_string[32 * 4096 + 1]; // +1 for the null terminator
        for (int i = 0; i < 32; ++i) {
            volatile char *page_start = global_array + i * 4096;
            char value = *page_start; // Accessing and reading each page
            char temp_str[2]; // To convert char to string
            temp_str[0] = value;
            temp_str[1] = '\0'; // Null terminate the temporary string
            strcat(result_string, temp_str); // Concatenate the character to the result string

        }
    } else if (which == 2) {
        volatile char *allocation = (char *)mmap((void *)global_array[4096*16], 1000*1000, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (allocation == MAP_FAILED) {
            fprintf(stderr, "Memory mapping failed\n");
            exit(1);
        }

        for (int i = 0; i < 1; ++i) {
            // Incremental access to demonstrate resident set size increase
            char bruh = allocation[i];
            printf("%c", bruh);
        }
    } else if (which == 3) {
        int RESIDENT_SET_SIZE_INCREMENT = (8 * 1024);
        volatile char *allocation = (char *)mmap(NULL, 1024*1024, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

        if (allocation == MAP_FAILED) {
            fprintf(stderr, "Memory mapping failed\n");
            exit(1);
        }

        // Perform the fewest memory accesses (you can choose not to access the allocated memory)

        // Increase resident set size by accessing pages within the allocated region
        for (int i = 0; i < RESIDENT_SET_SIZE_INCREMENT; ++i) {
            allocation[i] = 'A';
        }
    } else if (which == 4) {
        volatile char *allocation = mmap((void *)(0x5555555bbfff + 0x200000), 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (allocation == MAP_FAILED) {
            fprintf(stderr, "Memory mapping failed\n");
            exit(1);
        }

        // Access or use the allocated memory (read or write operations)
        for (int i = 0; i < 4096; ++i) {
            allocation[i] = 'B';
        }

        
        
    } else if (which ==5) {
        volatile char *allocation = mmap((void *)(0x5555555bbfff + 0x10000000000), 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (allocation == MAP_FAILED) {
            fprintf(stderr, "Memory mapping failed\n");
            exit(1);
        }

        // Access or use the allocated memory (read or write operations)
        for (int i = 0; i < 4096; ++i) {
            allocation[i] = 'C';
        }
    }else {
        return;
    }
}

int main(int argc, char **argv) {
    int which = 0;
    if (argc > 1) {
        which = atoi(argv[1]);
    } else {
        fprintf(stderr, "Usage: %s NUMBER\n", argv[0]);
        return 1;
    }
    printf("Memory layout:\n");
    print_maps(stdout);
    printf("\n");
    printf("Initial state:\n");
    force_load();
    struct memory_record r1, r2;
    record_memory_record(&r1);
    print_memory_record(stdout, NULL, &r1);
    printf("---\n");

    printf("Running labStuff(%d)...\n", which);

    labStuff(which);

    printf("---\n");
    printf("Afterwards:\n");
    record_memory_record(&r2);
    print_memory_record(stdout, &r1, &r2);
    print_maps(stdout);
    return 0;
}
