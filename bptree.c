#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ORDER 4  // Maximum number of children per internal node
#define MAX_KEYS (ORDER - 1)  // Maximum number of keys per node
#define MAX_CHILDREN (ORDER)  // Same as ORDER, maximum number of children

// Structure for B+ tree nodes
typedef struct BPNode {
    int keys[MAX_KEYS];
    struct BPNode* children[MAX_CHILDREN];
    struct BPNode* next;  // For leaf node linking
    int num_keys;
    int is_leaf;
} BPNode;

// Function prototypes
BPNode* create_node(int is_leaf);
BPNode* insert(BPNode* root, int key);
void insert_non_full(BPNode* node, int key);
void split_child(BPNode* parent, int index, BPNode* child);
BPNode* search(BPNode* root, int key);
void print_tree(BPNode* root, int level);
void search_and_print(BPNode* root, int key);
void run_example_1();
void run_example_2();
void run_example_3();
void run_example_4();
void print_usage();
void split_leaf_node(BPNode* parent, int index, BPNode* leaf);
void split_internal_node(BPNode* parent, int index, BPNode* internal);
BPNode* bulk_load(int* values, int n);

// Create a new node
BPNode* create_node(int is_leaf) {
    BPNode* new_node = (BPNode*)malloc(sizeof(BPNode));
    new_node->is_leaf = is_leaf;
    new_node->num_keys = 0;
    new_node->next = NULL;
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        new_node->children[i] = NULL;
    }
    
    return new_node;
}

// Search function
BPNode* search(BPNode* root, int key) {
    if (root == NULL) return NULL;
    
    // Always traverse to leaf node
    while (!root->is_leaf) {
        int i = 0;
        while (i < root->num_keys && key >= root->keys[i]) {
            i++;
        }
        root = root->children[i];
    }
    
    // Now at leaf node, search for the key
    for (int i = 0; i < root->num_keys; i++) {
        if (root->keys[i] == key) {
            return root;
        }
    }
    
    return NULL;
}

// Insert a key into the B+ tree
BPNode* insert(BPNode* root, int key) {
    if (root == NULL) {
        root = create_node(1);  // Create a new leaf node as root
        root->keys[0] = key;
        root->num_keys = 1;
        return root;
    }
    
    if (root->num_keys == MAX_KEYS) {
        BPNode* new_root = create_node(0);
        new_root->children[0] = root;
        split_child(new_root, 0, root);
        insert_non_full(new_root, key);
        return new_root;
    }
    
    insert_non_full(root, key);
    return root;
}

// Insert into a non-full node
void insert_non_full(BPNode* node, int key) {
    int i = node->num_keys - 1;
    
    if (node->is_leaf) {
        // Find the position to insert the new key
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        
        node->keys[i + 1] = key;
        node->num_keys++;
    } else {
        // Find the child which is going to have the new key
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        
        // Only split child if it's full
        if (node->children[i]->num_keys == MAX_KEYS) {
            split_child(node, i, node->children[i]);
            // After split, the new key might need to go into the next child
            if (key > node->keys[i]) {
                i++;
            }
        }
        
        insert_non_full(node->children[i], key);
    }
}

// Split a child node
void split_child(BPNode* parent, int index, BPNode* child) {
    if (child->is_leaf) {
        split_leaf_node(parent, index, child);
    } else {
        split_internal_node(parent, index, child);
    }
}

void split_leaf_node(BPNode* parent, int index, BPNode* leaf) {
    BPNode* new_leaf = create_node(1);  // Create new leaf node
    
    // When splitting a full node (3 keys) plus new key (total 4):
    // Left node gets ceil(4/2) = 2 keys
    // Right node gets floor(4/2) = 2 keys
    int K = MAX_KEYS;  // K = 3 in our case
    int split_point = (K + 1) / 2;  // ceil((K+1)/2) for left node
    
    // Copy the right portion of keys to the new leaf
    new_leaf->num_keys = K - split_point;  // floor((K+1)/2) keys for right node
    for (int i = 0; i < new_leaf->num_keys; i++) {
        new_leaf->keys[i] = leaf->keys[i + split_point];
    }
    leaf->num_keys = split_point;
    
    // Insert separator key into parent (first key of new leaf)
    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
        parent->keys[i] = parent->keys[i - 1];
    }
    parent->children[index + 1] = new_leaf;
    parent->keys[index] = new_leaf->keys[0];
    parent->num_keys++;
    
    // Link the leaf nodes
    new_leaf->next = leaf->next;
    leaf->next = new_leaf;
}

void split_internal_node(BPNode* parent, int index, BPNode* internal) {
    BPNode* new_internal = create_node(0);  // Create new internal node
    
    // For internal nodes, we follow the same formula but the middle key moves up
    int K = MAX_KEYS;
    int split_point = (K + 1) / 2;  // Middle key position
    
    // Copy the right portion of keys (after the middle key)
    new_internal->num_keys = K - split_point;
    for (int i = 0; i < new_internal->num_keys; i++) {
        new_internal->keys[i] = internal->keys[i + split_point];
    }
    
    // Copy the right portion of children
    for (int i = 0; i <= new_internal->num_keys; i++) {
        new_internal->children[i] = internal->children[i + split_point];
    }
    
    // Move up the middle key to the parent
    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
        parent->keys[i] = parent->keys[i - 1];
    }
    parent->children[index + 1] = new_internal;
    parent->keys[index] = internal->keys[split_point - 1];  // Middle key moves up
    parent->num_keys++;
    
    internal->num_keys = split_point - 1;  // Left node keeps keys before middle
}

// Print the tree (for debugging)
void print_tree(BPNode* root, int level) {
    if (root == NULL) return;
    
    // Print indentation
    for (int i = 0; i < level; i++) {
        printf("    ");  // 4 spaces for each level
    }
    
    // Print node type and keys
    if (root->is_leaf) {
        printf("Leaf [ ");
    } else {
        printf("Internal [ ");
    }
    
    // Print keys
    for (int i = 0; i < root->num_keys; i++) {
        printf("%d ", root->keys[i]);
    }
    printf("]");
    
    // Print leaf node links
    if (root->is_leaf && root->next != NULL) {
        printf(" -> next");
    }
    printf("\n");
    
    // Print children recursively
    if (!root->is_leaf) {
        for (int i = 0; i <= root->num_keys; i++) {
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
        for (int i = 0; i < result->num_keys; i++) {
            printf("%d ", result->keys[i]);
        }
        printf("\n");
    } else {
        printf("Key %d not found in the tree\n", key);
    }
}

void run_example_1() {
    BPNode* root = NULL;
    int test_values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
    int n = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("\nExample 1: Basic B+ Tree Operations\n");
    printf("Inserting values: 10, 20, 30, 40, 50, 60, 70, 80, 90\n");
    
    for(int i = 0; i < n; i++) {
        root = insert(root, test_values[i]);
        printf("\nAfter inserting %d:\n", test_values[i]);
        print_tree(root, 0);
        printf("------------------------\n");
    }
    
    // Test searches
    int search_keys[] = {30, 50, 90, 100};
    for (int i = 0; i < 4; i++) {
        search_and_print(root, search_keys[i]);
    }
}

void run_example_2() {
    BPNode* root = NULL;
    
    // Insert values in a pattern that allows for fuller nodes
    // First batch: multiples of 3
    for(int i = 3; i <= 99; i += 3) {
        root = insert(root, i);
    }
    
    // Second batch: multiples of 3 plus 1
    for(int i = 1; i <= 100; i += 3) {
        root = insert(root, i);
    }
    
    // Final batch: multiples of 3 plus 2
    for(int i = 2; i <= 98; i += 3) {
        root = insert(root, i);
    }
    
    printf("\nTree after inserting values in non-sequential order:\n");
    print_tree(root, 0);
}

void run_example_3() {
    printf("\nExample 3: Bulk Loading B+ Tree\n");
    
    // Create sorted array of values 1-100
    int values[100];
    for (int i = 0; i < 100; i++) {
        values[i] = i + 1;
    }
    
    // Bulk load the tree
    BPNode* root = bulk_load(values, 100);
    
    printf("\nTree after bulk loading values 1-100:\n");
    print_tree(root, 0);
    
    // Test some searches
    int search_keys[] = {1, 50, 100, 101};
    for (int i = 0; i < 4; i++) {
        search_and_print(root, search_keys[i]);
    }
}

void run_example_4() {
    BPNode* root = NULL;
    
    printf("\nExample 4: Sequential Insertion Limitation\n");
    printf("This example shows why inserting sequential values one-by-one\n");
    printf("leads to partially-filled nodes.\n\n");
    
    // Insert values 1-4 one at a time
    for(int i = 1; i <= 4; i++) {
        root = insert(root, i);
        printf("\nAfter inserting %d:\n", i);
        print_tree(root, 0);
        
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

void print_usage() {
    printf("Usage: bptree -e <example_number>\n");
    printf("Available examples:\n");
    printf("  1: Basic B+ Tree Operations (inserting 9 values)\n");
    printf("  2: Non-sequential Insertion Pattern\n");
    printf("  3: Bulk Loading (values 1-100)\n");
    printf("  4: Sequential Insertion Limitation\n");
}

BPNode* bulk_load(int* values, int n) {
    if (n == 0) return NULL;
    
    // Calculate number of leaf nodes needed
    int num_leaves = (n + MAX_KEYS - 1) / MAX_KEYS;  // Ceiling division
    
    // Allocate arrays for leaf nodes and internal nodes
    BPNode** leaf_nodes = malloc(num_leaves * sizeof(BPNode*));
    BPNode** current_level = malloc(num_leaves * sizeof(BPNode*));
    BPNode** next_level = malloc(num_leaves * sizeof(BPNode*));
    
    // Create and fill leaf nodes
    int leaf_idx = 0;
    BPNode* current_leaf = create_node(1);
    leaf_nodes[leaf_idx++] = current_leaf;
    
    for (int i = 0; i < n; i++) {
        if (current_leaf->num_keys == MAX_KEYS) {
            current_leaf = create_node(1);
            leaf_nodes[leaf_idx - 1]->next = current_leaf;
            leaf_nodes[leaf_idx++] = current_leaf;
        }
        current_leaf->keys[current_leaf->num_keys++] = values[i];
    }
    
    // Initialize current_level with leaf nodes
    for (int i = 0; i < leaf_idx; i++) {
        current_level[i] = leaf_nodes[i];
    }
    int current_size = leaf_idx;
    
    // Build internal nodes bottom-up
    while (current_size > 1) {
        int next_size = 0;
        int i = 0;
        
        while (i < current_size) {
            BPNode* parent = create_node(0);
            next_level[next_size++] = parent;
            
            int remaining = current_size - i;
            int children_to_add = (remaining > MAX_CHILDREN) ? MAX_CHILDREN : remaining;
            
            // Ensure remaining after this group has at least 2 children or take all
            if (remaining - children_to_add < 2 && remaining - children_to_add > 0) {
                children_to_add = remaining;
            }
            
            // Add children to parent
            for (int j = 0; j < children_to_add; j++) {
                parent->children[j] = current_level[i + j];
                if (j > 0) {
                    parent->keys[j - 1] = current_level[i + j]->keys[0];
                    parent->num_keys++;
                }
            }
            i += children_to_add;
        }
        
        // Update current_level and current_size for next iteration
        current_size = next_size;
        for (int k = 0; k < current_size; k++) {
            current_level[k] = next_level[k];
        }
    }
    
    BPNode* root = (current_size == 1) ? current_level[0] : NULL;
    
    free(leaf_nodes);
    free(current_level);
    free(next_level);
    
    return root;
}

int main(int argc, char* argv[]) {
    if (argc != 3 || strcmp(argv[1], "-e") != 0) {
        print_usage();
        return 1;
    }
    
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
        default:
            printf("Invalid example number: %d\n", example);
            print_usage();
            return 1;
    }
    
    return 0;
}
