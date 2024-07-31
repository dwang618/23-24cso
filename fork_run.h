#ifndef FUNCTIONS_H
#define FUNCTIONS_H

char *getoutput(const char *command);
char *parallelgetoutput(int count, const char **argv_base);

#endif  // FUNCTIONS_H