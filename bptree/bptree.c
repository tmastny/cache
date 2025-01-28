#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// We'll use the definition of ORDER defined here:
// https://cs186berkeley.net/notes/note4/
#ifndef ORDER
#define ORDER 2  // Default order if not specified during compilation
#endif
#define MAX_KEYS (2 * ORDER)  // Maximum number of keys per node
#define MAX_CHILDREN (2 * ORDER + 1)  // Same as ORDER, maximum number of children

typedef enum NodeType {
    LEAF,
    INTERNAL
} NodeType;

typedef struct BPNode {
    int keys[MAX_KEYS + 1];
    struct BPNode* children[MAX_CHILDREN + 1];
    struct BPNode* next;  // For leaf node linking
    int nkeys;
    NodeType type;
} BPNode;

typedef struct BPTree {
    BPNode* root;
} BPTree;

BPTree bptree;

BPNode* node_new(NodeType type) {
    BPNode* new_node = (BPNode*)malloc(sizeof(BPNode));
    new_node->type = type;
    new_node->nkeys = 0;
    new_node->next = NULL;
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        new_node->children[i] = NULL;
    }

    for (int i = 0; i < MAX_KEYS; i++) {
        new_node->keys[i] = 0;
    }
    
    return new_node;
}

void bptree_init(BPTree* bptree) {
    bptree->root = node_new(LEAF);
}

typedef struct Search {
    BPNode* node;
    int index;
} Search;

Search bptree_search(BPTree* bptree, int key) {
    BPNode* node = bptree->root;
    while (node->type != LEAF) {
        int i = 0;
        while (i < node->nkeys && key >= node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }

    for (int i = 0; i < node->nkeys; i++) {
        if (node->keys[i] == key) {
            return (Search){node, i};
        }
    }
    
    return (Search){NULL, -1};
}

void node_insert_entry(BPNode* node, int key, BPNode* child) {
    int i = 0;
    while (key >= node->keys[i] && i < node->nkeys) {
        i++;
    }
    
    for (int j = node->nkeys; j > i; j--) {
        node->keys[j] = node->keys[j - 1];
        node->children[j + 1] = node->children[j];
    }
    
    node->keys[i] = key;
    node->children[i + 1] = child;
    node->nkeys++;
}

typedef struct Split {
    BPNode* right;
    int key;
} Split;

Split node_split(BPNode* node, int key) {
    BPNode* new_node = node_new(node->type);

    Split split;
    split.right = new_node;
    split.key = node->keys[node->nkeys / 2];

    // If the node is an internal node, we need to MOVE
    // the first key to the parent node.
    int key_copy_start = node->nkeys / 2;
    if (node->type == INTERNAL) {
        key_copy_start++;
    }

    for (int i = key_copy_start; i < node->nkeys; i++) {
        new_node->keys[new_node->nkeys++] = node->keys[i];
        new_node->children[new_node->nkeys - 1] = node->children[i];
        node->keys[i] = 0;
        node->children[i] = NULL;
    }

    new_node->children[new_node->nkeys] = node->children[node->nkeys];

    node->keys[node->nkeys / 2] = 0;
    node->children[node->nkeys / 2 + 1] = NULL;
    node->children[node->nkeys] = NULL;

    node->nkeys = node->nkeys / 2;

    return split;
}

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

void print_tree(BPNode* root, int level);

void bptree_insert(BPTree* bptree, int key) {
    node_insert(bptree->root, key, NULL);
}

void print_tree(BPNode* root, int level) {
    if (root == NULL) return;
    
    for (int i = 0; i < level; i++) {
        printf("    ");  // 4 spaces for each level
    }
    
    if (root->type == LEAF) {
        printf("Leaf [ ");
    } else {
        printf("Internal [ ");
    }
    
    for (int i = 0; i < root->nkeys; i++) {
        printf("%d ", root->keys[i]);
    }
    printf("]");
    
    if (root->type == LEAF && root->next != NULL) {
        printf(" -> next");
    }
    printf("\n");
    
    if (root->type != LEAF) {
        for (int i = 0; i <= root->nkeys; i++) {
            print_tree(root->children[i], level + 1);
        }
    }
}

void search_and_print(BPTree* bptree, int key) {
    printf("\nSearching for %d:\n", key);

    Search result = bptree_search(bptree, key);

    if (result.index == -1) {
        printf("Key %d not found in the tree\n", key);
        return;
    }

    printf("Found key %d at index %d in leaf node with keys:\n  ", key, result.index);
    for (int i = 0; i < result.node->nkeys; i++) {
        printf("%d ", result.node->keys[i]);
    }
    printf("\n");
}

typedef struct Parent {
    BPNode* node;
    struct Parent* parent;
} Parent;

Parent* parent_new(BPNode* parent_node, Parent* parent, BPNode* left_child) {
    Parent* p = (Parent*)malloc(sizeof(Parent));
    p->node = parent_node;
    p->parent = parent;

    if (left_child != NULL) {
        p->node->children[0] = left_child;
    }

    return p;
}

void parent_free(Parent* p) {
    while (p != NULL) {
        Parent* next = p->parent;
        free(p);
        p = next;
    }
}

void parent_insert(Parent* p, int key, BPNode* child) {
    node_insert_entry(p->node, key, child);
    
    // child's parent does not change
    if (p->node->nkeys <= MAX_KEYS) {
        return;
    }

    Split split = node_split(p->node, key);

    if (p->parent == NULL) {
        bptree.root = node_new(INTERNAL);
        p->parent = parent_new(bptree.root, NULL, p->node);
    }

    parent_insert(p->parent, split.key, split.right);
    p->node = split.right;
}

void bptree_bulk_insert(BPTree* bptree, int* values, int n) {
    Parent leaf_parent;
    leaf_parent.node = node_new(INTERNAL);
    leaf_parent.parent = NULL;
    bptree->root = leaf_parent.node;

    bool insert_left_child = true;

    int i = 0;
    while (i < n) {
        BPNode* leaf = node_new(LEAF);
        
        while (i < n && leaf->nkeys < MAX_KEYS) {
            node_insert_entry(leaf, values[i], NULL);
            i++;
        }

        if (insert_left_child) {
            leaf_parent.node->children[0] = leaf;
            insert_left_child = false;
            continue;
        } 

        parent_insert(&leaf_parent, leaf->keys[0], leaf);
    }

    parent_free(leaf_parent.parent);
}

void run_example_1() {
    BPNode* root = NULL;
    int test_values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
    int n = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("\nExample 1: Basic B+ Tree Operations\n");
    printf("Inserting values: 10, 20, 30, 40, 50, 60, 70, 80, 90\n");
    
    for(int i = 0; i < n; i++) {
        bptree_insert(&bptree, test_values[i]);
        printf("\nAfter inserting %d:\n", test_values[i]);
        print_tree(bptree.root, 0);
        printf("------------------------\n");
    }
    
    int search_keys[] = {30, 50, 90, 100};
    for (int i = 0; i < 4; i++) {
        search_and_print(&bptree, search_keys[i]);
    }
}

void run_example_2() {
    
    // Insert values in a pattern that allows for fuller nodes
    // First batch: multiples of 3
    for(int i = 3; i <= 99; i += 3) {
        printf("inserting %d\n", i);
        bptree_insert(&bptree, i);
    }
    
    // Second batch: multiples of 3 plus 1
    for(int i = 1; i <= 100; i += 3) {
        bptree_insert(&bptree, i);
    }
    
    // Final batch: multiples of 3 plus 2
    for(int i = 2; i <= 98; i += 3) {
        bptree_insert(&bptree, i);
    }
    
    printf("\nTree after inserting values in non-sequential order:\n");
    print_tree(bptree.root, 0);
}

void run_example_3() {
    printf("\nExample 3: Bulk Loading B+ Tree\n");
    
    // Create sorted array of values 1-100
    int values[100];
    for (int i = 0; i < 100; i++) {
        values[i] = i + 1;
    }
    
    // Bulk load the tree
    bptree_bulk_insert(&bptree, values, 100);
    
    printf("\nTree after bulk loading values 1-100:\n");
    print_tree(bptree.root, 0);
    
    // Test some searches
    int search_keys[] = {1, 50, 100, 101};
    for (int i = 0; i < 4; i++) {
        search_and_print(&bptree, search_keys[i]);
    }
}

void run_example_4() {
    printf("\nExample 4: Sequential Insertion Limitation\n");
    printf("This example shows why inserting sequential values one-by-one\n");
    printf("leads to partially-filled nodes.\n\n");
    
    // Insert values 1-4 one at a time
    for(int i = 1; i <= 4; i++) {
        bptree_insert(&bptree, i);
        printf("\nAfter inserting %d:\n", i);
        print_tree(bptree.root, 0);
        
        if (i == 3) {
            printf("Note: Leaf node is now full with [1 2 3]\n");
        }
        if (i == 4) {
            printf("\nNote: When 4 is inserted, we must split:\n");
            printf("- Left leaf gets [1 2]\n");
            printf("- Right leaf gets [3 4]\n");
            printf("- Parent gets [3]\n");
            printf("\nThere's no way to get a third key into [1 2] because\n");
            printf("any new key would be >= 3!\n");
        }
        printf("------------------------\n");
    }
}

void run_example_5() {
    int test_values[] = {10, 20, 30, 25};
    
    printf("\nExample 5: Non-Sequential Insertion Pattern\n");
    printf("Inserting values in order: 10, 20, 30, 25\n");
    
    for(int i = 0; i < 4; i++) {
        bptree_insert(&bptree, test_values[i]);
        printf("\nAfter inserting %d:\n", test_values[i]);
        print_tree(bptree.root, 0);
        printf("------------------------\n");
    }
    
    // Test searches
    printf("\nTesting searches:\n");
    for (int i = 0; i < 4; i++) {
        search_and_print(&bptree, test_values[i]);
    }
}

void run_example_10() {
    int test_values[] = {10, 20, 30, 40, 50};
    
    printf("\nExample 10: Simple Sequential Insertion\n");
    printf("Inserting values in order: 10, 20, 30, 40\n");
    
    for(int i = 0; i < 4; i++) {
        bptree_insert(&bptree, test_values[i]);

        printf("\nAfter inserting %d:\n", test_values[i]);
        print_tree(bptree.root, 0);
        printf("------------------------\n");
    }
}

void run_example_11() {
    int test_values[] = {10, 20, 30, 40, 25, 26, 29};
    int n = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("\nExample 11: Mixed Sequential and Non-Sequential Insertion\n");
    printf("Inserting values: 10, 20, 30, 40, 25, 26, 29\n");
    
    for(int i = 0; i < n; i++) {
        bptree_insert(&bptree, test_values[i]);
        printf("\nAfter inserting %d:\n", test_values[i]);
        print_tree(bptree.root, 0);
        printf("------------------------\n");
    }
}

void run_example_12() {
    int test_values[] = {10, 20, 30, 40, 25};
    int n = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("\nExample 12: Mixed Sequential and Non-Sequential Insertion\n");
    printf("Inserting values: 10, 20, 30, 40, 25\n");
    
    for(int i = 0; i < n; i++) {
        bptree_insert(&bptree, test_values[i]);
        printf("\nAfter inserting %d:\n", test_values[i]);
        print_tree(bptree.root, 0);
        printf("------------------------\n");
    }
}

int bptree_height(BPTree* bptree) {
    if (bptree->root == NULL) return 0;
    
    int height = 1;
    BPNode* node = bptree->root;
    while (node->type != LEAF) {
        height++;
        node = node->children[0];  // Follow leftmost path
    }
    return height;
}

double bptree_avg_keys(BPTree* bptree) {
    if (bptree->root == NULL) return 0;
    
    // Calculate max possible nodes for 100M elements
    // At minimum 50% full, each leaf has ORDER keys
    // So number of leaves ≈ N/ORDER
    // Total nodes will be less than 2 * number of leaves
    size_t max_nodes = (2 * 100000000) / ORDER + 1;
    BPNode** queue = malloc(sizeof(BPNode*) * max_nodes);
    if (queue == NULL) {
        printf("Failed to allocate queue\n");
        return 0;
    }
    
    int total_nodes = 0;
    int total_keys = 0;
    int front = 0, rear = 0;
    queue[rear++] = bptree->root;
    
    while (front < rear) {
        BPNode* node = queue[front++];
        total_nodes++;
        total_keys += node->nkeys;
        
        if (node->type != LEAF) {
            for (int i = 0; i <= node->nkeys; i++) {
                queue[rear++] = node->children[i];
            }
        }
    }
    
    // double result = (double)total_keys / total_nodes;
    double result = (double)total_keys / (total_nodes * ORDER);
    free(queue);
    return result;
}

void example_100() {
    const int N = 100000000;  // 100M elements
    const int SEARCHES = 1000000;  // 1M searches
    
    printf("Order: %d\n", ORDER);
    
    int* values = (int*)malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++) {
        values[i] = i;
    }
    
    clock_t start = clock();
    bptree_bulk_insert(&bptree, values, N);
    clock_t end = clock();
    double bulk_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Measure tree characteristics
    int height = bptree_height(&bptree);
    double avg_keys = bptree_avg_keys(&bptree);
    printf("Tree height: %d\n", height);
    printf("Average keys per node: %.2f\n", avg_keys);
    
    free(values);
    
    // Rest of the search benchmark code...
    start = clock();
    int found = 0;
    for (int i = 0; i < SEARCHES; i++) {
        int key = rand() % N;
        Search result = bptree_search(&bptree, key);
        if (result.node != NULL) {
            found++;
        }
    }
    end = clock();
    
    double search_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    double avg_search_time = (search_time * 1000000.0) / SEARCHES;
    
    printf("Average search time: %.2f microseconds\n", avg_search_time);
}

void print_usage() {
    printf("Usage: bptree -e <example_number>\n");
    printf("Available examples:\n");
    printf("  1: Basic B+ Tree Operations (inserting 9 values)\n");
    printf("  2: Non-sequential Insertion Pattern\n");
    printf("  3: Bulk Loading (values 1-100)\n");
    printf("  4: Sequential Insertion Limitation\n");
    printf("  5: Inserting 10, 20, 30, 25\n");
    printf("  10: Simple Sequential Insertion (10, 20, 30, 40)\n");
    printf("  11: Mixed Sequential/Non-Sequential (10, 20, 30, 40, 25, 26, 29)\n");
    printf("  12: Mixed Sequential/Non-Sequential (10, 20, 30, 40, 25)\n");
    printf("  100: Bulk Loading and Random Searches\n");
}

int main(int argc, char* argv[]) {
    if (argc != 3 || strcmp(argv[1], "-e") != 0) {
        print_usage();
        return 1;
    }
    
    bptree_init(&bptree);
    
    int example = atoi(argv[2]);
    
    switch(example) {
        case 1:
            run_example_1();
            break;
        case 2:
            run_example_2();
            break;
        case 3:
            run_example_3();
            break;
        case 4:
            run_example_4();
            break;
        case 5:
            run_example_5();
            break;
        case 10:
            run_example_10();
            break;
        case 11:
            run_example_11();
            break;
        case 12:
            run_example_12();
            break;
        case 100:
            example_100();
            break;
        default:
            printf("Invalid example number: %d\n", example);
            print_usage();
            return 1;
    }
    
    return 0;
}
