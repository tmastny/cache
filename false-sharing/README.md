# false-sharing

Operation                | Approximate Cost
------------------------ | ----------------
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
2       | 0.029   | 0.029           | -1.02%
4       | 0.031   | 0.031           | -0.78%
6       | 0.063   | 0.037           | 41.66%
8       | 0.079   | 0.041           | 48.06%

Multi Lock (multilock.c)
Threads | Regular | 128-byte Padded | Improvement
--------|---------|-----------------|-------------
2       | 0.369   | 0.369           | 0.15%
4       | 0.228   | 0.224           | 1.68%
6       | 0.343   | 0.201           | 41.47%
8       | 0.321   | 0.163           | 49.40%

Atomic Operations (atomic.c)
Threads | Regular | 128-byte Padded | Improvement
--------|---------|-----------------|-------------
2       | 1.362   | 0.189           | 620.1%
4       | 4.201   | 0.193           | 2079.8%
6       | 18.735  | 0.210           | 8821.5%
8       | 20.378  | 0.227           | 8890.4%