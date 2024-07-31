#include <stdlib.h>
#include <string.h>
#include "split.h"

char **string_split(const char *input, const char *sep, int *num_words) {
    char** retArr = NULL;
    
    if(input ==NULL || sep==NULL || num_words == NULL) {
        return NULL;
    }

    size_t ind = 0;
    int wordcnt = 0;
    while(input[ind]) {
        //skip separators, filter through first characters falling within sep to next word
        size_t nextWordDistance = strspn(&input[ind], sep);
        if (ind == 0 && nextWordDistance > 0) {
            retArr = realloc(retArr, sizeof(char *) * (wordcnt + 1));
            if (retArr == NULL) {
                // handle allocation failure
                for (int x = 0; x < wordcnt; x++) {
                    free(retArr[x]);
                }
                free(retArr);
                return NULL;
        }
        else if (nextWordDistance == 0) {
            retArr = realloc(retArr, sizeof(char *) * (wordcnt + 1));
            if (retArr == NULL) {
                // handle allocation failure
                for (int x = 0; x < wordcnt; x++) {
                    free(retArr[x]);
                }
                free(retArr);
                return NULL;
        }

        retArr[wordcnt] = malloc(1);  // allocate memory for one character (empty word)
        if (retArr[wordcnt] == NULL) {
            for (int x = 0; x < wordcnt; x++) {
                free(retArr[x]);
            }
            free(retArr);
            return NULL;
        }
    
    retArr[wordcnt][0] = '\0';  // null-terminate the string
    ind++;
    wordcnt++;
}
    
        else {
            ind += nextWordDistance;
        }

        //find end of current word before proceeding delimiter
        size_t currentWordLen = strcspn(&input[ind], sep);

        //check that current word is not a space(s)
        if(currentWordLen >= 1) {

            //reposition result array to fit size of new word
            char** nextArr = realloc(retArr, sizeof(char *) * (wordcnt+1));
            if(nextArr == NULL) {
                for(int x=0; x<wordcnt; x++) {
                    free(retArr[x]);
                }
                free(retArr);
                return NULL;
            }
            retArr = nextArr;

             //alloc memory for new word and copy it
            retArr[wordcnt] = malloc(currentWordLen + 1);
            if(retArr[wordcnt] == NULL) {
                for(int x=0; x<wordcnt; x++) {
                    free(retArr[x]);
                }
                free(retArr);
                return NULL;
            }
            //  dynamic array repositioned to new address with appropriate size, following word is made at other location
            // take copy of word, add null terminator to mark individual word in array
            strncpy(retArr[wordcnt], &input[ind], currentWordLen);
            retArr[wordcnt][currentWordLen] = '\0';
            ind = currentWordLen + ind; 
            wordcnt++;

        }
        else {
            //printf("no");
            ind++;
        }

    }

    *num_words = wordcnt;
    return retArr;

}



