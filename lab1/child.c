#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#define MAX_BUFFER 1024


int check_overflow(const char *str_number) {
    char *endptr;
    errno = 0;
    long result = strtol(str_number, &endptr, 10);

    if ((result == LONG_MAX || result == LONG_MIN) && errno == ERANGE)
        return 0;

    else if (*endptr != '\0' || result > INT_MAX || result < INT_MIN)
        return 0;

    return 1;
}



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
            int check_of_overflow = 0;
            
            if (check_overflow(token)) check_of_overflow += 1;
            num1 = atoi(token);

            while ((token = strtok(NULL, " ")) != NULL) {
                if (check_overflow(token)) check_of_overflow += 1;
                num2 = atoi(token);

                if (num2 == 0) {
                    fprintf(stderr, "Ошибка: деление на 0\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
                int result = num1 / num2;

                if (check_of_overflow == 2) printf("%d / %d = %d\n", num1, num2, result);
                else printf("Ошибка: переполнение\n");
            }
        }
    }

    fclose(file);
    return 0;
}