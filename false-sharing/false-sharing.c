#include <stdio.h>
#include <omp.h>
#include <pthread.h>
#include <unistd.h>  // for usleep

// Regular mutexes (potential false sharing)
pthread_mutex_t regular_locks[16];

// 64-byte padded mutexes
struct padded_mutex_64 {
    pthread_mutex_t mutex;
    char padding[64 - sizeof(pthread_mutex_t)];
} __attribute__((aligned(64)));
struct padded_mutex_64 padded_locks_64[16];

// 128-byte padded mutexes
struct padded_mutex_128 {
    pthread_mutex_t mutex;
    char padding[128 - sizeof(pthread_mutex_t)];
} __attribute__((aligned(128)));
struct padded_mutex_128 padded_locks_128[16];

// Some work to do while holding lock
void do_work() {
    // Small, consistent amount of work
    volatile int x = 0;
    for(int i = 0; i < 100; i++) {
        x += i;
    }
}

double test_locks(int padding_type) {
    double start = omp_get_wtime();
    
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int lock_index = id % 16;
        
        // Each thread hammers its own lock
        for(int i = 0; i < 1000000; i++) {
            switch(padding_type) {
                case 0: // regular
                    pthread_mutex_lock(&regular_locks[lock_index]);
                    do_work();
                    pthread_mutex_unlock(&regular_locks[lock_index]);
                    break;
                case 64: // 64-byte padding
                    pthread_mutex_lock(&padded_locks_64[lock_index].mutex);
                    do_work();
                    pthread_mutex_unlock(&padded_locks_64[lock_index].mutex);
                    break;
                case 128: // 128-byte padding
                    pthread_mutex_lock(&padded_locks_128[lock_index].mutex);
                    do_work();
                    pthread_mutex_unlock(&padded_locks_128[lock_index].mutex);
                    break;
            }
        }
    }
    
    return omp_get_wtime() - start;
}

int main() {
    // Initialize locks
    for(int i = 0; i < 16; i++) {
        pthread_mutex_init(&regular_locks[i], NULL);
        pthread_mutex_init(&padded_locks_64[i].mutex, NULL);
        pthread_mutex_init(&padded_locks_128[i].mutex, NULL);
    }
    
    // Run tests with different thread counts
    for(int threads = 2; threads <= 8; threads *= 2) {
        printf("\nTesting with %d threads:\n", threads);
        omp_set_num_threads(threads);
        
        double regular_total = 0, padded_64_total = 0, padded_128_total = 0;
        
        int iterations = 100;
        for(int iter = 0; iter < iterations; iter++) {
            regular_total += test_locks(0);
            padded_64_total += test_locks(64);
            padded_128_total += test_locks(128);
        }
        
        printf("Average time with regular locks: %f seconds\n", 
               regular_total / (float)iterations);
        printf("Average time with 64-byte padded locks: %f seconds\n", 
               padded_64_total / (float)iterations);
        printf("Average time with 128-byte padded locks: %f seconds\n", 
               padded_128_total / (float)iterations);
        printf("Difference (regular vs 64-byte): %f seconds\n", 
               (regular_total - padded_64_total) / (float)iterations);
        printf("Difference (regular vs 128-byte): %f seconds\n", 
               (regular_total - padded_128_total) / (float)iterations);
    }
    
    return 0;
}