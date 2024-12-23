#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>

#define MAX_BUFFER 1024

int main() {
    int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, MAX_BUFFER);
    char *shared_memory = mmap(0, MAX_BUFFER, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    sem_t *sem = sem_open("/my_semaphore", O_CREAT, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid != 0) {
        sem_wait(sem);

        printf("The result from the child process: %s\n", shared_memory);

        munmap(shared_memory, MAX_BUFFER);
        shm_unlink("/my_shm");
        sem_close(sem);
        sem_unlink("/my_semaphore");
        wait(NULL);
    } else {
        char filename[256];
        printf("Input file name: ");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = 0;

        snprintf(shared_memory, MAX_BUFFER, "%s", filename);

        sem_post(sem);

        char *args[] = {"./child", filename, NULL};
        execv(args[0], args);
        perror("execv failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}