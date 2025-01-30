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

// Reduce work to make cache effects more prominent
void do_work() {
    volatile int x = 0;
    x += 1;
}

double test_locks(int padding_type) {
    double start = omp_get_wtime();
    
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        // Always use adjacent locks to maximize false sharing effect
        int lock_index = id;  
        
        for(int i = 0; i < 2000000; i++) {
            switch(padding_type) {
                case 0:
                    pthread_mutex_lock(&regular_locks[lock_index]);
                    do_work();
                    pthread_mutex_unlock(&regular_locks[lock_index]);
                    break;
                case 64:
                    pthread_mutex_lock(&padded_locks_64[lock_index].mutex);
                    do_work();
                    pthread_mutex_unlock(&padded_locks_64[lock_index].mutex);
                    break;
                case 128:
                    pthread_mutex_lock(&padded_locks_128[lock_index].mutex);
                    do_work();
                    pthread_mutex_unlock(&padded_locks_128[lock_index].mutex);
                    break;
            }
            
            // Add tiny delay between operations to reduce contention
            for(volatile int j = 0; j < 10; j++);
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
    for(int threads = 2; threads <= 8; threads += 2) {
        printf("\nTesting with %d threads:\n", threads);
        omp_set_num_threads(threads);
        
        double regular_total = 0, padded_64_total = 0, padded_128_total = 0;
        
        // Increase number of measurements
        int iterations = 100;  // Increased from 20
        for(int iter = 0; iter < iterations; iter++) {
            regular_total += test_locks(0);
            padded_64_total += test_locks(64);
            padded_128_total += test_locks(128);
            
            // Print progress
            if ((iter + 1) % 10 == 0) {
                printf("Completed %d iterations...\n", iter + 1);
            }
        }
        
        printf("Average time with regular locks: %f seconds\n", 
               regular_total / (float)iterations);
        printf("Average time with 64-byte padded locks: %f seconds\n", 
               padded_64_total / (float)iterations);
        printf("Average time with 128-byte padded locks: %f seconds\n", 
               padded_128_total / (float)iterations);
        printf("Difference (regular vs 64-byte): %f seconds (%.2f%%)\n", 
               (regular_total - padded_64_total) / (float)iterations,
               100.0 * (regular_total - padded_64_total) / regular_total);
        printf("Difference (regular vs 128-byte): %f seconds (%.2f%%)\n", 
               (regular_total - padded_128_total) / (float)iterations,
               100.0 * (regular_total - padded_128_total) / regular_total);
    }
    
    return 0;
}