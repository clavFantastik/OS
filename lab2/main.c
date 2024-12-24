#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

typedef struct thread_data {
    char *word;
    char *string;
    int start;
    int end;
    int found;
} thread_data;

void *naive_search(void *arg) {
    thread_data *data = (thread_data *)arg;

    int m = strlen(data->word);

    for (int i = data->start; i < data->end; i++) {
        int flag = 0;
        for (int j = 0; j < m; j++) {
            if (data->string[i + j] == data->word[j]) {
                flag = 1;
            }
            else {
                flag = 0;
                break;
            }
        }
        if (flag) {
            data->found = i;
            break;
        }
    }
    return NULL;
}

char *get_string() {
    int len = 0;
    int capacity = 1; 
    char *s = (char*) malloc(sizeof(char));

    char c = getchar();

    while (c != '\n') {
        s[len++] = c; 
        char * for_realloc;

        if (len >= capacity) {
            capacity *= 2; 
            for_realloc = (char*) realloc(s, capacity * sizeof(char));

            if(!for_realloc) {
                free(s);
                return NULL;
            }
            s = for_realloc;
        }

        c = getchar();
    }

    s[len] = '\0';

    return s;
}


int main(int argc, char *argv[]) {
    int threads_num = strtol(argv[1], NULL, 10);

    pthread_t threads[threads_num];
    thread_data threads_data[threads_num];

    size_t size = 1000000000;
    char *string = (char *)malloc(size * sizeof(char));

    if (string == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    memset(string, 'A', size);

    int place = 82346;

    string[place + 3] = 'l'; string[place + 2] = 'a'; string[place + 1] = 'o'; string[place] = 'g';
    printf("Enter search word: ");
    char *word = get_string();

    int part_size = strlen(string) / threads_num;
    int part_remains = strlen(string) % threads_num; 

    clock_t start_time = clock();

    for (int i = 0; i < threads_num; i++) {
        threads_data[i].string = string; 
        threads_data[i].word = word;
        threads_data[i].found = INT_MAX;

        if(part_remains) {
            threads_data[i].start = i * part_size + 1;
            part_remains--;
        }
        else threads_data[i].start = i * part_size;
        
        threads_data[i].end = threads_data[i].start + part_size + strlen(word);

        if(pthread_create(&threads[i], NULL, naive_search, &threads_data[i])) {
            free(string); free(word);
            return 1;
        }
    }

    int min_ind = INT_MAX;

    for(int i = 0; i < threads_num; i++) {
        pthread_join(threads[i], NULL);
        if (threads_data[i].found < min_ind) 
            min_ind = threads_data[i].found;
    }

    clock_t end_time = clock();
    
    double delta_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;


    if (min_ind != INT_MAX)
        printf("Goal number is: %d\n", min_ind);
    else
        printf("Word hasn't been found\n");

    printf("Working time with %d thread(s) is %f s.\n", threads_num, delta_time);

    free(word);
    free(string);
    return 0;
}