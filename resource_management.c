#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_RESOURCES 5
#define NUM_THREADS 5

int available_resources = MAX_RESOURCES;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int decrease_count(int thread_number, int count) {
    pthread_mutex_lock(&mutex);
    int success = 0;
    if (available_resources < count) {
        success = -1;
    } else {
        available_resources -= count;
    }
    pthread_mutex_unlock(&mutex);

    if (success == 0) {
        printf("The thread %d has acquired %d resources; %d more resources are available.\n", 
               thread_number, count, available_resources);
    }
    return success;
}

int increase_count(int thread_number, int count) {
    pthread_mutex_lock(&mutex);
    available_resources += count;
    printf("The thread %d has released, %d resources and %d resources are now available.\n", 
           thread_number, count, available_resources);
    pthread_mutex_unlock(&mutex);
    return 0;
}

void* thread_function(void* arg) {
    int thread_number = *(int*)arg;
    int resources_to_acquire = 1;
    
    while (decrease_count(thread_number, resources_to_acquire) != 0) {
        printf("Thread %d could not acquire enough resources\n", thread_number);
        sleep(1);
    }
    
    sleep(1);
    
    increase_count(thread_number, resources_to_acquire);
    
    free(arg);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int i;
    
    for (i = 0; i < NUM_THREADS; i++) {
        int* thread_num = malloc(sizeof(int));
        if (thread_num == NULL) {
            fprintf(stderr, "Failed to allocate memory\n");
            return 1;
        }
        *thread_num = i;
        if (pthread_create(&threads[i], NULL, thread_function, (void*)thread_num) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            free(thread_num);
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("All threads have finished execution. Available resources: %d\n", available_resources);
    
    pthread_mutex_destroy(&mutex);
    
    return 0;
}