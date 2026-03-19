#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define SUCCESS 0
#define OPERATION_FAILED 1
#define INVALID_INPUT 2

typedef struct Node {
    uint32_t data;
    struct Node* first_child;
    struct Node* next_sibling;
    struct Node* parent;
} Node;

typedef struct Tree {
    Node* root;
} Tree;

Node* create_node(uint32_t data, Node* parent) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        return NULL;
    }
    new_node->data = data;
    new_node->first_child = NULL;
    new_node->next_sibling = NULL;
    new_node->parent = parent;
    return new_node;
}

Node* find_node (Node* root, uint32_t node_data) {
    if (root == NULL) {
        printf("root can't be NULL\n");
        return NULL;
    }
    if (root->data == node_data) {
        return root;
    }

    Node* child = root->first_child;
    while (child != NULL) {
        Node* result = find_node(child, node_data);
        if (result != NULL) {
            return result;
        }
        child = child->next_sibling;
    }

    return NULL; 
}

int add_node(Node* root, uint32_t child_data, uint32_t parent_data) {
    if (root == NULL) {
        printf("Tree can't be null\n");
    }
    if (find_node(root, child_data) != NULL) {
        printf("Node with such key already exists\n");
        return INVALID_INPUT;
    }

    Node* parent = find_node(root, parent_data);

    if (parent == NULL) {
        printf("Node with such key doesn't exist\n");
        return INVALID_INPUT;
    }

    Node* child = create_node(child_data, parent);

    if (parent->first_child == NULL) {
        parent->first_child = child;
    } else {
        Node* last_sibling = parent->first_child;
        while (last_sibling->next_sibling != NULL) {
            last_sibling = last_sibling->next_sibling;
        }
        last_sibling->next_sibling = child;
    }
    return SUCCESS;
}

int delete_node(Node* node) {
    Node* child = node->first_child;
    while (child != NULL) {
        Node* next = child->next_sibling;
        delete_node(child);
        child = next;
    }
    
    if (node->parent != NULL) {
        Node* parent = node->parent;
        if (parent->first_child == node) {
            parent->first_child = node->next_sibling;
        } else {
            Node* prev = parent->first_child;
            while (prev != NULL && prev->next_sibling != node) {
                prev = prev->next_sibling;
            }
            if (prev != NULL) {
                prev->next_sibling = node->next_sibling;
            }
        }
    }
    
    node->parent = NULL;
    node->next_sibling = NULL;
    node->first_child = NULL;
    free(node);
    return SUCCESS;
}


int delete_by_key(Node* root, uint32_t node_data) {
    if (root == NULL) {
        return INVALID_INPUT;
    }

    Node* node = find_node(root, node_data);
    if (node == NULL) {
        printf("Can't delete NULL node\n");
        return INVALID_INPUT;
    }

    return delete_node(node);
}

int create_tree(uint32_t root_data, Tree** tree_var) {
    Node* root = create_node(root_data, NULL);
    if (root == NULL) {
        printf("Something went wrong\n");
        return OPERATION_FAILED;
    }
    Tree* tree = (Tree*)malloc(sizeof(Tree));
    if (tree == NULL) {
        free(root);
        printf("Something went wrong\n");
        return OPERATION_FAILED;
    }

    tree->root = root;
    *tree_var = tree;

    return SUCCESS;
}

uint32_t max_key_depth(Node* node, uint32_t* curr_max, uint32_t* max_depth, uint32_t curr_depth) {
    if (*curr_max == 0 || node->data > *curr_max) {
        *curr_max = node->data;
        *max_depth = curr_depth;
    }

    Node* child = node->first_child;
    while (child != NULL) {
        max_key_depth(child, curr_max, max_depth, curr_depth + 1);
        child = child->next_sibling;
    }

    return *max_depth;
}

int find_max_depth(Node* node) {
    if (node == NULL) {
        printf("Tree can't be NULL\n");
        return INVALID_INPUT;
    }

    uint32_t curr_max = 0;
    uint32_t max_depth = 0;

    uint32_t ans = max_key_depth(node, &curr_max, &max_depth, 0);
    printf("%u\n", ans);
    return SUCCESS;
}



void print_tree(Node* root, int depth) {
    if (root == NULL) {
        printf("Tree can't be NULL\n");
        return;
    }
    
    for(int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%d\n", root->data);
    
    Node* child = root->first_child;
    while (child != NULL) {
        print_tree(child, depth + 1);
        child = child->next_sibling;
    }
}

int delete_tree(Tree* tree) {
    if (tree == NULL || tree->root == NULL) {
        printf("Can't delete a NULL tree\n");
        return INVALID_INPUT;
    }
    delete_node(tree->root);
    tree->root = NULL;
    free(tree);
    return SUCCESS;
}

void cli() {
    Tree* tree = NULL;
    char option;
    int status;
    
    while (1) {
        printf("Choose an option: \n");
        printf("1. Create tree\n");
        printf("2. Add node\n");
        printf("3. Delete node\n");
        printf("4. Max depth\n");
        printf("5. Print tree\n");
        printf("6. Delete tree\n");
        printf("q. Quit\n");
        printf("Choice: ");
        
        
        scanf(" %c", &option);

        switch (option) {
        case '1':
            if (tree != NULL) {
                printf("Tree already exists");
                break;
            }
            uint32_t root_key;
            printf("Enter root key: ");
            scanf("%u", &root_key);
            if (create_tree(root_key, &tree) == SUCCESS) {
                printf("Tree created with root %u\n", root_key);
            }
            break;
            
        case '2':
            if (tree == NULL) {
                printf("Tree can't be NULL\n");
                break;
            }
            printf("Child key: ");
            uint32_t child_key;
            scanf("%u", &child_key);
            printf("Parent key: ");
            uint32_t parent_key;
            scanf("%u", &parent_key);
            
            if (add_node(tree->root, child_key, parent_key) == SUCCESS) {
                printf("Node %u added with parent: %u\n", child_key, parent_key);
            }
            break;
        case '3':
            if (tree == NULL) {
                printf("Tree can't be NULL\n");
                break;
            }
            uint32_t key;
            printf("Node key to delete: ");
            scanf("%u", &key);
            if (delete_by_key(tree->root, key) == SUCCESS) {
                printf("Node with key %d deleted\n", key);
            }
            break;
        case '4':
            if (tree == NULL) {
                printf("Tree can't be NULL\n");
                break;
            }
            find_max_depth(tree->root);
            break;
        case '5':
            if (tree == NULL) {
                printf("Tree can't be NULL\n");
                break;
            }
            print_tree(tree->root, 0);
            break;
        case '6':
            if (tree != NULL) {
                delete_tree(tree);
                tree = NULL;
                printf("Tree deleted\n");
            }
            break;
        case 'q':
        case 'Q':
            if (delete_tree(tree) == SUCCESS) {
                tree = NULL;
                printf("Tree deleted\n");
                return;
            }
            break;
        default:
            printf("Invalid option. Use 1-6\n");
        }
    }
    
    if (tree != NULL) {
        delete_tree(tree);
    }
}


int main() {
    cli();
    return 0;
}