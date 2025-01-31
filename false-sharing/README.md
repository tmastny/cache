# false-sharing

Operation                | Approximate Cost
------------------------ | ----------------
Non-atomic Add           | ~1-3 cycles
Atomic Add               | ~5-50 cycles
Syscall Entry            | ~100-1000 cycles
Mutex Lock (uncontended) | ~200-2000 cycles
Context Switch           | ~1000-10000 cycles
Mutex Lock (contended)   | ~2000-20000 cycles

Uncontended: requires at least one syscall (lock)
Contended: requires syscall + context switch + another syscall when waking up

Mutex operations can easily mask the false sharing effect. 

## Results

Single Lock (false-sharing.c)
Threads | Regular | 128-byte Padded | Improvement
--------|---------|-----------------|-------------
2       | 0.029   | 0.030          | -4.19%
4       | 0.030   | 0.032          | -4.04%
6       | 0.064   | 0.037          | 42.99%
8       | 0.072   | 0.042          | 41.90%

Multi Lock (multilock.c)
Threads | Regular | 128-byte Padded | Improvement
--------|---------|-----------------|-------------
2       | 0.367   | 0.367          | 0.22%
4       | 0.222   | 0.219          | 1.41%
6       | 0.335   | 0.197          | 41.18%
8       | 0.309   | 0.160          | 48.22%

Atomic Operations (atomic.c)
Threads | Regular | 128-byte Padded | Improvement
--------|---------|-----------------|-------------
2       | 1.362   | 0.189           | 620.1%
4       | 4.201   | 0.193           | 2079.8%
6       | 18.735  | 0.210           | 8821.5%
8       | 20.378  | 0.227           | 8890.4%

Simple Counter (counter.c)
Threads | Regular | 128-byte Padded | Improvement
--------|---------|-----------------|-------------
2       | 0.019   | 0.018           | 5.06%
4       | 0.021   | 0.019           | 7.97%
6       | 0.028   | 0.020           | 25.84%
8       | 0.030   | 0.023           | 25.12%

ab (ab.c)
Average time with regular variables: 0.019053 seconds
Average time with padded variables: 0.018923 seconds
Difference: 0.000130 seconds (0.68%)
