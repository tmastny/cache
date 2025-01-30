#include <stdio.h>
#include <omp.h>
#include <stdatomic.h>
#include <pthread.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>

#define CACHE_LINE_SIZE 128
#define NUM_THREADS 8  
#define ITERATIONS 100000000

// Pin thread to specific core (0-3 for P cores)
void pin_to_core(int core_id) {
    thread_port_t thread = pthread_mach_thread_np(pthread_self());
    thread_affinity_policy_data_t policy = { core_id };
    thread_policy_set(thread, THREAD_AFFINITY_POLICY, 
                     (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
}

// Regular counters (potential false sharing)
struct Regular { atomic_int counter; };
struct Regular regular[NUM_THREADS];

// Padded counters (each in own cache line)
struct Padded { 
    atomic_int counter;
    char padding[CACHE_LINE_SIZE - sizeof(atomic_int)]; 
} __attribute__((aligned(CACHE_LINE_SIZE)));
struct Padded padded[NUM_THREADS];

double test_counters(int use_padding) {
    double start = omp_get_wtime();
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        pin_to_core(id);  // Pin to P cores 0-3
        
        if (use_padding) {
            for(int i=0; i<ITERATIONS; i++)
                atomic_fetch_add_explicit(&padded[id].counter, 1, memory_order_relaxed);
        } else {
            for(int i=0; i<ITERATIONS; i++)
                atomic_fetch_add_explicit(&regular[id].counter, 1, memory_order_relaxed);
        }
    }
    return omp_get_wtime() - start;
}

int main() {
    printf("Testing false sharing on P cores:\n");
    
    for(int threads=2; threads<=NUM_THREADS; threads+=2) {
        omp_set_num_threads(threads);
        printf("\n%d threads:\n", threads);
        
        // Warmup
        test_counters(0);
        test_counters(1);
        
        // Timed runs
        double regular_time = test_counters(0);
        double padded_time = test_counters(1);
        
        printf("Regular: %.3fs  Padded: %.3fs\n", regular_time, padded_time);
        printf("Speedup: %.1f%%\n", 
              (regular_time/padded_time - 1) * 100);
    }
    
    return 0;
}