#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Не указано имя файла\n");
        return 1;
    }
    
    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Не удалось открыть файл");
        return 1;
    }

    char line[MAX_BUFFER];
    while (fgets(line, sizeof(line), file)) {
        int num1, num2;
        char *token = strtok(line, " ");
        if (token != NULL) {
            num1 = atoi(token);
            while ((token = strtok(NULL, " ")) != NULL) {
                num2 = atoi(token);
                if (num2 == 0) {
                    fprintf(stderr, "Ошибка: деление на 0\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                int result = num1 / num2;
                printf("%d / %d = %d\n", num1, num2, result);
            }
        }
    }

    fclose(file);
    return 0;
}