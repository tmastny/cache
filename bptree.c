#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// We'll use the definition of ORDER defined here:
// https://cs186berkeley.net/notes/note4/
#define ORDER 2 
#define MAX_KEYS (2 * ORDER)  // Maximum number of keys per node
#define MAX_CHILDREN (2 * ORDER + 1)  // Same as ORDER, maximum number of children

typedef enum NodeType {
    LEAF,
    INTERNAL
} NodeType;

// Structure for B+ tree nodes
// Add extra key to support splits
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


// Create a new node
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

// Search function
BPNode* search(BPNode* node, int key) {
    while (node->type != LEAF) {
        int i = 0;
        while (i < node->nkeys && key >= node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }
    
    return node;
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

// Insert a key into the B+ tree
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

// Print the tree (for debugging)
void print_tree(BPNode* root, int level) {
    if (root == NULL) return;
    
    // Print indentation
    for (int i = 0; i < level; i++) {
        printf("    ");  // 4 spaces for each level
    }
    
    // Print node type and keys
    if (root->type == LEAF) {
        printf("Leaf [ ");
    } else {
        printf("Internal [ ");
    }
    
    // Print keys
    for (int i = 0; i < root->nkeys; i++) {
        printf("%d ", root->keys[i]);
    }
    printf("]");
    
    // Print leaf node links
    if (root->type == LEAF && root->next != NULL) {
        printf(" -> next");
    }
    printf("\n");
    
    // Print children recursively
    if (root->type != LEAF) {
        for (int i = 0; i <= root->nkeys; i++) {
            print_tree(root->children[i], level + 1);
        }
    }
}

// Let's also add a helper function to make testing easier
void search_and_print(BPNode* root, int key) {
    printf("\nSearching for %d:\n", key);
    BPNode* result = search(root, key);
    if (result != NULL) {
        printf("Found key %d in leaf node with keys: ", key);
        for (int i = 0; i < result->nkeys; i++) {
            printf("%d ", result->keys[i]);
        }
        printf("\n");
    } else {
        printf("Key %d not found in the tree\n", key);
    }
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
        if (i == 20) {
            printf("inserting %d\n", values[i]);
        }
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
        

        print_tree(bptree->root, 0);
    }
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
    
    // Test searches
    int search_keys[] = {30, 50, 90, 100};
    for (int i = 0; i < 4; i++) {
        search_and_print(bptree.root, search_keys[i]);
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
        search_and_print(bptree.root, search_keys[i]);
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
        search_and_print(bptree.root, test_values[i]);
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
        default:
            printf("Invalid example number: %d\n", example);
            print_usage();
            return 1;
    }
    
    return 0;
}
