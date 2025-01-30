#include <stdio.h>
#include <omp.h>
#include <pthread.h>
#include <stdint.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>

#define CACHE_LINE_SIZE 128

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

int main() {
    // Regular variables (adjacent in memory)
    uint16_t a = 0;
    uint16_t b = 0;
    
    // Padded variables (on different cache lines)
    uint16_t a_padded = 0;
    char padding[CACHE_LINE_SIZE - sizeof(uint16_t)];
    uint16_t b_padded = 0;
    
    double regular_total = 0, padded_total = 0;
    int iterations = 100;
    
    for(int iter = 0; iter < iterations; iter++) {
        // Test regular variables
        double start = omp_get_wtime();
        #pragma omp parallel num_threads(2)
        {
            int id = omp_get_thread_num();
            pin_to_core(id);
            do_work(id == 0 ? &a : &b);
        }
        regular_total += omp_get_wtime() - start;
        
        // Test padded variables
        start = omp_get_wtime();
        #pragma omp parallel num_threads(2)
        {
            int id = omp_get_thread_num();
            pin_to_core(id);
            do_work(id == 0 ? &a_padded : &b_padded);
        }
        padded_total += omp_get_wtime() - start;
        
        if ((iter + 1) % 10 == 0) {
            printf("Completed %d iterations...\n", iter + 1);
        }
    }
    
    printf("\nResults with 2 threads:\n");
    printf("Average time with regular variables: %f seconds\n", 
           regular_total / (float)iterations);
    printf("Average time with padded variables: %f seconds\n", 
           padded_total / (float)iterations);
    printf("Difference: %f seconds (%.2f%%)\n", 
           (regular_total - padded_total) / (float)iterations,
           100.0 * (regular_total - padded_total) / regular_total);
    
    return 0;
}
