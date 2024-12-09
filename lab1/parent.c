#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER 1024

int main() {
    int pipe1[2];
    if (pipe(pipe1) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid != 0) {
        close(pipe1[1]);

        char buffer[MAX_BUFFER];
        while (read(pipe1[0], buffer, sizeof(buffer)) > 0) {
            printf("Результат из дочернего процесса: %s", buffer);
        }

        close(pipe1[0]);
        wait(NULL); 
    } else { 
        close(pipe1[0]);

        char filename[256];
        printf("Введите имя файла: ");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = 0;

        
        char *args[] = {"./child", filename, NULL};
        dup2(pipe1[1], STDOUT_FILENO); 
        close(pipe1[1]); 

        execv(args[0], args);
        perror("execv failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}