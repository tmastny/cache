#!/bin/bash

for order in 2 3 10 15 60 100 120 240 500 1000; do
    echo "Testing with ORDER = $order"
    gcc -DORDER=$order bptree.c -o bptree
    
    ./bptree -e 100
    
    echo "----------------------------------------"
done 