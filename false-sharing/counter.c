#include <stdio.h>
#include <omp.h>
#include <pthread.h>
#include <stdint.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>

#define NUM_THREADS 8
#define CACHE_LINE_SIZE 128

// Regular array (all counters in same cache line)
struct regular_counter {
    uint16_t value;
} __attribute__((aligned(2)));  // align to uint16_t boundary
struct regular_counter regular[NUM_THREADS];

// Padded array (each counter in its own cache line)
struct padded_counter {
    uint16_t value;
    char padding[CACHE_LINE_SIZE - sizeof(uint16_t)];
} __attribute__((aligned(CACHE_LINE_SIZE)));
struct padded_counter padded[NUM_THREADS];

// Pin thread to specific core
void pin_to_core(int core_id) {
    thread_port_t thread = pthread_mach_thread_np(pthread_self());
    thread_affinity_policy_data_t policy = { core_id };
    thread_policy_set(thread, THREAD_AFFINITY_POLICY, 
                     (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
}

void do_work(uint16_t* counter) {
    for(int i = 0; i < 10000000; i++) {
        (*counter)++;
    }
}

double test_counters(int use_padding) {
    double start = omp_get_wtime();
    
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        pin_to_core(id);
        
        if (use_padding) {
            do_work(&padded[id].value);
        } else {
            do_work(&regular[id].value);
        }
    }
    
    return omp_get_wtime() - start;
}

int main() {
    // Initialize arrays
    for(int i = 0; i < NUM_THREADS; i++) {
        regular[i].value = 0;
        padded[i].value = 0;
    }
    
    // Run tests with different thread counts
    for(int threads = 2; threads <= 8; threads += 2) {
        printf("\nTesting with %d threads:\n", threads);
        omp_set_num_threads(threads);
        
        double regular_total = 0, padded_total = 0;
        
        int iterations = 100;
        for(int iter = 0; iter < iterations; iter++) {
            regular_total += test_counters(0);
            padded_total += test_counters(1);
            
            if ((iter + 1) % 10 == 0) {
                printf("Completed %d iterations...\n", iter + 1);
            }
        }
        
        printf("Average time with regular array: %f seconds\n", 
               regular_total / (float)iterations);
        printf("Average time with padded array: %f seconds\n", 
               padded_total / (float)iterations);
        printf("Difference: %f seconds (%.2f%%)\n", 
               (regular_total - padded_total) / (float)iterations,
               100.0 * (regular_total - padded_total) / regular_total);
    }
    
    return 0;
}