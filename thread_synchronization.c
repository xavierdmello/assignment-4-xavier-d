#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

#define MAX_THREADS 100

typedef struct thread {
    char id[4];
    int start_time;
    int y; 
} thread_t;

thread_t threads[MAX_THREADS];
int thread_count = 0;

sem_t mutex;
sem_t odd_sem, even_sem;
int last_y_parity = -1; 

void* threadRun(void* arg) {
    thread_t* t = (thread_t*)arg;

    if (t->y % 2 == 0) {
        sem_wait(&even_sem);
    } else {
        sem_wait(&odd_sem);
    }

    sem_wait(&mutex);


    printf("[Thread %s] is in its critical section\n", t->id);
    sleep(1);

    sem_post(&mutex);

  
    if (last_y_parity == 0) {
        sem_post(&odd_sem);
    } else if (last_y_parity == 1) {
        sem_post(&even_sem);
    }

    last_y_parity = t->y % 2; 

    printf("[Thread %s] has finished\n", t->id);
    return NULL;
}

int compareStartTime(const void* a, const void* b) {
    thread_t* t1 = (thread_t*)a;
    thread_t* t2 = (thread_t*)b;
    return t1->start_time - t2->start_time;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Failed to open input file");
        return 1;
    }

    while (fscanf(file, "%s %d", threads[thread_count].id, &threads[thread_count].start_time) != EOF) {
        threads[thread_count].y = threads[thread_count].id[2] - '0';
        thread_count++;
    }
    fclose(file);

  
    qsort(threads, thread_count, sizeof(thread_t), compareStartTime);


    sem_init(&odd_sem, 0, 0);
    sem_init(&even_sem, 0, 0);

    pthread_t thread_ids[MAX_THREADS];


    for (int i = 0; i < thread_count; i++) {
        if (i == 0) {
            last_y_parity = threads[i].y % 2;
            sem_post(last_y_parity == 0 ? &even_sem : &odd_sem);
        }

        pthread_create(&thread_ids[i], NULL, threadRun, &threads[i]);
        sleep(threads[i].start_time);
    }

    // Join threads
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    // Destroy semaphores
    sem_destroy(&mutex);
    sem_destroy(&odd_sem);
    sem_destroy(&even_sem);

    return 0;
}
