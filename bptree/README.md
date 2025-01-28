# bptree

Incomplete implementation of B+ tree used to study 
the relationship between the order of the tree and search time,
especially as it relates to cache and random access speeds.

## Features

One thing I like about this implementation is that
insert is not recursive, but managed by a stack.

After looking at several different implementations,
I thought this was clean and easy to follow, for me at least. 

```c
void node_insert(BPNode* node, int key, BPNode* child) {
    BPNode* stack[100];
    int top = 1;
    stack[0] = NULL;
    while (node->type != LEAF) {
        int i = 0;
        while (i < node->nkeys && key >= node->keys[i]) {
            i++;
        }
        stack[top++] = node;
        node = node->children[i];
    }

    while (top > 0) {
        BPNode* parent = stack[top - 1];
        node_insert_entry(node, key, child);
        if (node->nkeys <= MAX_KEYS) {
            return;
        }
        Split split = node_split(node, key);
        key = split.key;
        child = split.right;

        if (parent == NULL) {
            BPNode* parent = node_new(INTERNAL);
            parent->children[0] = bptree.root;
            node_insert_entry(parent, key, child);
            bptree.root = parent;
            return;
        }

        node = stack[--top];
    }
}
```

Implemented:
- Insertion
- Bulk insertion
- Search

Missing:
- Scanning
- Deletion


## Benchmark

See `bptree_bench.sh`.

## Results

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

Formula:
- Time = (height × pointer_chase_cost) + (height × avg_keys_per_node/2 × comparison_cost)

Let's calculate for a few key points:

ORDER = 2
- Height: 17 predicted, 16 actual
- Avg keys/node: 4 predicted, 3.33 actual
- Time = (17 × 100ns) + (17 × 2 × 1ns)
- 1700ns + 34ns
- ≈ 1.15 microseconds
- Actual: 1.85, 0.72 microseconds

ORDER = 3
- Height: 15 predicted, 13 actual
- Avg keys/node: 6 predicted, 5.25 actual
- Time = (15 × 100ns) + (15 × 3 × 1ns)
- 1500ns + 45ns
- ≈ 1.02 microseconds
- Actual: 0.72, 0.64 microseconds

ORDER = 10
- Height: 10 predicted, 8 actual
- Avg keys/node: 20 predicted, 19.09 actual
- Time = (10 × 100ns) + (10 × 10 × 1ns)
- 1000ns + 100ns
- ≈ 0.73 microseconds
- Actual: 0.46, 0.43 microseconds

ORDER = 15
- Height: 9 predicted, 7 actual
- Avg keys/node: 30 predicted, 29.06 actual
- Time = (9 × 100ns) + (9 × 15 × 1ns)
- 900ns + 135ns
- ≈ 0.67 microseconds
- Actual: 0.44, 0.41 microseconds

ORDER = 60
- Height: 6 predicted, 5 actual
- Avg keys/node: 120 predicted, 119.02 actual
- Time = (6 × 100ns) + (6 × 60 × 1ns)
- 600ns + 360ns
- ≈ 0.58 microseconds
- Actual: 0.41, 0.38 microseconds

ORDER = 100
- Height: 6 predicted, 4 actual
- Avg keys/node: 200 predicted, 199.01 actual
- Time = (6 × 100ns) + (6 × 100 × 1ns)
- 600ns + 600ns
- ≈ 0.70 microseconds
- Actual: 0.45, 0.43 microseconds

ORDER = 120
- Height: 6 predicted, 4 actual
- Avg keys/node: 240 predicted, 239.01 actual
- Time = (6 × 100ns) + (6 × 120 × 1ns)
- 600ns + 720ns
- ≈ 0.76 microseconds
- Actual: 0.50, 0.46 microseconds

ORDER = 240
- Height: 5 predicted, 4 actual
- Avg keys/node: 480 predicted, 479.00 actual
- Time = (5 × 100ns) + (5 × 240 × 1ns)
- 500ns + 1200ns
- ≈ 0.93 microseconds
- Actual: 0.75, 0.72 microseconds

ORDER = 500
- Height: 5 predicted, 3 actual
- Avg keys/node: 1000 predicted, 999.00 actual
- Time = (5 × 100ns) + (5 × 500 × 1ns)
- 500ns + 2500ns
- ≈ 1.59 microseconds
- Actual: 1.09, 1.07 microseconds

ORDER = 1000
- Height: 4 predicted, 3 actual
- Avg keys/node: 2000 predicted, 1999.00 actual
- Time = (4 × 100ns) + (4 × 1000 × 1ns)
- 400ns + 4000ns
- ≈ 2.27 microseconds
- Actual: 1.97, 1.80 microseconds

```bash
order,actual,expected
2,1.85,1.15
3,0.72,1.02
10,0.46,0.73
15,0.44,0.67
60,0.41,0.58
100,0.45,0.70
120,0.50,0.76
240,0.75,0.93
500,1.09,1.59
1000,1.97,2.27
```
