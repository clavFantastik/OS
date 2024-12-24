#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_BUFFER 8192

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

    int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666);
    char *shared_memory = mmap(0, MAX_BUFFER, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    shared_memory[0] = '\0';
    char buffer[128];
    char string[MAX_BUFFER];
    string[0] = '\0';

    sem_t *sem = sem_open("/my_semaphore1", 0);
    if (sem == SEM_FAILED) {
        perror("sem_open failed 3");
        exit(EXIT_FAILURE);
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

            if (check_overflow(token)) {
                    snprintf(buffer, 128, "Error: int overflow in token = %s\n", token);
                    strcat(string, buffer);
            }
            else {
                num1 = atoi(token);

                while ((token = strtok(NULL, " ")) != NULL) {
                    if (check_overflow(token)) {
                        snprintf(buffer, 128, "Error: int overflow in token = %s\n", token);
                        strcat(string, buffer);
                    }
                    else {
                        num2 = atoi(token);
                        
                        if (num2 == 0) {
                            snprintf(buffer, 128, "Error: division by zero, exit();\n");

                            strcat(string, buffer);
                            strcpy(shared_memory, string);
                            sem_post(sem);

                            munmap(shared_memory, MAX_BUFFER);
                            shm_unlink("/my_semaphore1");
                            sem_close(sem);
                            exit(EXIT_FAILURE);
                        }
                        
                        int result = num1 / num2;
                        snprintf(buffer, 128, "%d / %d = %d\n", num1, num2, result);
                        strcat(string, buffer);
                        
                    }
                    
                }
            }
        }
    }

    strcpy(shared_memory, string);
    sem_post(sem);
    munmap(shared_memory, MAX_BUFFER);
    shm_unlink("/my_semaphore1");
    sem_close(sem);

    fclose(file);
    return 0;
}
