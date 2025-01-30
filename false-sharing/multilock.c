#include <stdio.h>
#include <omp.h>
#include <pthread.h>

// Regular mutexes (potential false sharing)
pthread_mutex_t regular_locks[16];

// 128-byte padded mutexes
struct padded_mutex_128 {
    pthread_mutex_t mutex;
    char padding[128 - sizeof(pthread_mutex_t)];
} __attribute__((aligned(128)));
struct padded_mutex_128 padded_locks_128[16];

// Minimal work to maximize cache effects visibility
void do_work() {
    volatile int x = 0;
    x += 1;
}

double test_locks(int padding_type) {
    double start = omp_get_wtime();
    
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        
        // Each thread takes every nth lock
        for(int i = 0; i < 10000000; i++) {
            // Lock and unlock multiple locks in sequence
            for(int lock_index = id; lock_index < 16; lock_index += num_threads) {
                if (padding_type == 0) {
                    pthread_mutex_lock(&regular_locks[lock_index]);
                    do_work();
                    pthread_mutex_unlock(&regular_locks[lock_index]);
                } else {
                    pthread_mutex_lock(&padded_locks_128[lock_index].mutex);
                    do_work();
                    pthread_mutex_unlock(&padded_locks_128[lock_index].mutex);
                }
            }
        }
    }
    
    return omp_get_wtime() - start;
}

int main() {
    // Initialize locks
    for(int i = 0; i < 16; i++) {
        pthread_mutex_init(&regular_locks[i], NULL);
        pthread_mutex_init(&padded_locks_128[i].mutex, NULL);
    }
    
    // Run tests with different thread counts
    for(int threads = 2; threads <= 8; threads += 2) {
        printf("\nTesting with %d threads:\n", threads);
        omp_set_num_threads(threads);
        
        double regular_total = 0, padded_total = 0;
        
        int iterations = 100;
        for(int iter = 0; iter < iterations; iter++) {
            regular_total += test_locks(0);
            padded_total += test_locks(1);
            
            if ((iter + 1) % 10 == 0) {
                printf("Completed %d iterations...\n", iter + 1);
            }
        }
        
        printf("Average time with regular locks: %f seconds\n", 
               regular_total / (float)iterations);
        printf("Average time with 128-byte padded locks: %f seconds\n", 
               padded_total / (float)iterations);
        printf("Difference: %f seconds (%.2f%%)\n", 
               (regular_total - padded_total) / (float)iterations,
               100.0 * (regular_total - padded_total) / regular_total);
    }
    
    return 0;
}
