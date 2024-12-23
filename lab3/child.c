#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/shm.h>
#include <semaphore.h>

#define MAX_BUFFER 1024

int check_overflow(const char *str_number) {
    if (!str_number) return 1;

    char *endptr;
    errno = 0; 
    long result = strtol(str_number, &endptr, 10);

    if (endptr == str_number) return 1;
    if ((result == LONG_MAX || result == LONG_MIN) && errno == ERANGE) return 1;  
    if (*endptr != '\0' || result > INT_MAX || result < INT_MIN)  return 1;  
    
    return 0;  
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: no file name\n");
        return 1;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error: file can't be open");
        return 1;
    }

    char line[MAX_BUFFER];
    while (fgets(line, sizeof(line), file)) {
        int num1, num2;
        char *token = strtok(line, " ");
        if (token != NULL) {

            if (check_overflow(token)) printf("Error: int overflow in token = %s\n", token);
            else {
                num1 = atoi(token);

                while ((token = strtok(NULL, " ")) != NULL) {
                    if (check_overflow(token)) printf("Error: int overflow in token = %s\n", token);
                    else {
                        num2 = atoi(token);

                        if (num2 == 0) {
                            fprintf(stderr, "Error: division by zero, exit();\n");
                            fclose(file);
                            exit(EXIT_FAILURE);
                        }
                        int result = num1 / num2;

                        printf("%d / %d = %d\n", num1, num2, result);
                    }
                    
                }
            }
        }
    }

    fclose(file);
    return 0;
}
