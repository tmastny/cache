#!/bin/bash

for order in 240; do
    echo "Testing with ORDER = $order"
    gcc -DORDER=$order bptree.c -o bptree
    
    ./bptree -e 100
    
    echo "----------------------------------------"
done 