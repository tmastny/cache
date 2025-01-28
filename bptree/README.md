Testing with ORDER = 2
Average search time: 1.85 microseconds
----------------------------------------
Testing with ORDER = 3
Average search time: 0.72 microseconds
----------------------------------------
Testing with ORDER = 10
Average search time: 0.46 microseconds
----------------------------------------
Testing with ORDER = 15
Average search time: 0.44 microseconds
----------------------------------------
Testing with ORDER = 60
Average search time: 0.41 microseconds
----------------------------------------
Testing with ORDER = 100
Average search time: 0.45 microseconds
----------------------------------------
Testing with ORDER = 120
Average search time: 0.50 microseconds
----------------------------------------
Testing with ORDER = 240
Average search time: 0.75 microseconds
----------------------------------------
Testing with ORDER = 500
Average search time: 1.09 microseconds
----------------------------------------
Testing with ORDER = 1000
Average search time: 1.97 microseconds
----------------------------------------


Base assumptions:
- Pointer chase (cache miss): ~100ns
- Sequential int comparison: ~1ns (much faster than our previous estimate due to CPU pipelining)
- Average case looks at half the keys in each node

Formula:
- Time = (height × pointer_chase_cost) + (height × avg_keys_per_node/2 × comparison_cost)

Let's calculate for a few key points:

ORDER = 2
- Height: 17 predicted, 16 actual
- Avg keys/node: 3 predicted, 3.33 actual
- Time = (17 × 100ns) + (17 × 2 × 1ns)
- 1700ns + 34ns
- ≈ 1.73, 1.15 microseconds
- Actual: 1.85, 0.72 microseconds

ORDER = 3
- Height: 15 predicted, 13 actual
- Avg keys/node: 4.5 predicted, 5.25 actual
- Time = (15 × 100ns) + (15 × 3 × 1ns)
- 1500ns + 45ns
- ≈ 1.53, 1.02 microseconds
- Actual: 0.72, 0.64 microseconds

ORDER = 10
- Height: 10 predicted, 8 actual
- Avg keys/node: 15 predicted, 19.09 actual
- Time = (10 × 100ns) + (10 × 10 × 1ns)
- 1000ns + 100ns
- ≈ 1.08, 0.73 microseconds
- Actual: 0.46, 0.43 microseconds

ORDER = 15
- Height: 9 predicted, 7 actual
- Avg keys/node: 22.5 predicted, 29.06 actual
- Time = (9 × 100ns) + (9 × 15 × 1ns)
- 900ns + 135ns
- ≈ 1.00, 0.67 microseconds
- Actual: 0.44, 0.41 microseconds

ORDER = 60
- Height: 6 predicted, 5 actual
- Avg keys/node: 90 predicted, 119.02 actual
- Time = (6 × 100ns) + (6 × 60 × 1ns)
- 600ns + 360ns
- ≈ 0.87, 0.58 microseconds
- Actual: 0.41, 0.38 microseconds

ORDER = 100
- Height: 6 predicted, 4 actual
- Avg keys/node: 150 predicted, 199.01 actual
- Time = (6 × 100ns) + (6 × 100 × 1ns)
- 600ns + 600ns
- ≈ 1.05, 0.70 microseconds
- Actual: 0.45, 0.43 microseconds

ORDER = 120
- Height: 6 predicted, 4 actual
- Avg keys/node: 180 predicted, 239.01 actual
- Time = (6 × 100ns) + (6 × 120 × 1ns)
- 600ns + 720ns
- ≈ 1.14, 0.76 microseconds
- Actual: 0.50, 0.46 microseconds

ORDER = 240
- Height: 5 predicted, 4 actual
- Avg keys/node: 360 predicted, 479.00 actual
- Time = (5 × 100ns) + (5 × 240 × 1ns)
- 500ns + 1200ns
- ≈ 1.40, 0.93 microseconds
- Actual: 0.75, 0.72 microseconds

ORDER = 500
- Height: 5 predicted, 3 actual
- Avg keys/node: 750 predicted, 999.00 actual
- Time = (5 × 100ns) + (5 × 500 × 1ns)
- 500ns + 2500ns
- ≈ 2.38, 1.59 microseconds
- Actual: 1.09, 1.07 microseconds

ORDER = 1000
- Height: 4 predicted, 3 actual
- Avg keys/node: 1500 predicted, 1999.00 actual
- Time = (4 × 100ns) + (4 × 1000 × 1ns)
- 400ns + 4000ns
- ≈ 3.40, 2.27 microseconds
- Actual: 1.97, 1.80 microseconds

```bash
order,actual,expected
2,1.85,1.73
3,0.72,1.53
10,0.46,1.08
15,0.44,1.00
60,0.41,0.87
100,0.45,1.05
120,0.50,1.14
240,0.75,1.40
500,1.09,2.38
1000,1.97,3.40
```
