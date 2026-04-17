#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define INITIAL_CAPACITY 8
#define MAX_TOKEN_LEN 64

typedef enum {
    OPERAND,
    OPERATOR,
    OPEN_PARENT,
    CLOSE_PARENT,
} TokenType;

typedef struct token {
    TokenType type;
    char* value;
} token;

typedef struct Node {
    token* data;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct OpStack {
    token** data;
    uint32_t capacity;
    uint32_t size;
} OpStack;

Node* create_node(token* data) {
    Node* node = malloc(sizeof(Node));
    if (node == NULL) {
        return NULL;
    }
    node->left = NULL;
    node->right = NULL;
    node->data = data;
    return node;
}

OpStack* create_op_stack(uint32_t capacity) {
    OpStack* stk = malloc(sizeof(OpStack));
    if (stk == NULL) return NULL;
    stk->data = malloc(capacity * sizeof(token*));
    if (stk->data == NULL) { 
        free(stk); 
        return NULL; 
    }
    stk->capacity = capacity;
    stk->size = 0;
    return stk;
}

int32_t push_op(OpStack* stk, token* input) {
    if (stk == NULL) {
        return EXIT_FAILURE;
    }
    if (stk->size >= stk->capacity) {
        stk->capacity *= 2;
        stk->data = realloc(stk->data, stk->capacity * sizeof(token*));
        if (stk->data == NULL) return EXIT_FAILURE;
    }
    stk->data[stk->size++] = input;
    return EXIT_SUCCESS;
}

token* pop_op(OpStack* stk) {
    if (stk == NULL || stk->data == NULL || stk->size == 0) {
        return NULL;
    }
    stk->size--;
    return stk->data[stk->size];
}

token* peek_op(OpStack* stk) {
    if (stk == NULL || stk->data == NULL || stk->size == 0) {
        return NULL;
    }
    return stk->data[stk->size - 1]; 
}

void free_op_stack(OpStack* stk) {
    if (stk == NULL) {
        return;
    }
    free(stk->data);
    free(stk);
}

typedef struct NodeStack {
    Node** data;
    uint32_t capacity;
    uint32_t size;
} NodeStack;

NodeStack* create_node_stack(uint32_t capacity) {
    NodeStack* stk = malloc(sizeof(NodeStack));
    if (stk == NULL) return NULL;
    stk->data = malloc(capacity * sizeof(Node*));
    if (stk->data == NULL) { 
        free(stk); 
        return NULL;
    }
    stk->capacity = capacity;
    stk->size = 0;
    return stk;
}

int32_t push_node(NodeStack* stk, Node* input) {
    if (stk == NULL) {
        return EXIT_FAILURE;
    }
    if (stk->size >= stk->capacity) {
        stk->capacity *= 2;
        stk->data = realloc(stk->data, stk->capacity * sizeof(Node*));
        if (stk->data == NULL) {
            return EXIT_FAILURE;
        }
    }
    stk->data[stk->size++] = input;
    return EXIT_SUCCESS;
}

Node* pop_node(NodeStack* stk) {
    if (stk == NULL || stk->data == NULL || stk->size == 0) return NULL;
    stk->size--;
    return stk->data[stk->size];
}

void free_tree(Node* root) {
    if (root == NULL) {
        return;
    }

    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

void free_node_stack(NodeStack* stk) {
    if (stk == NULL) {
        return;
    }

    for (uint32_t i = 0; i < stk->size; i++) {
        free_tree(stk->data[i]);
    }

    free(stk->data);
    free(stk);
}

void free_tokens(token* tokens, int32_t count) {
    for (int32_t i = 0; i < count; i++) {
        free(tokens[i].value);
    }
    free(tokens);
}

token* tokenize(char* expr, int32_t count) {
    int32_t capacity = INITIAL_CAPACITY;
    token* tokens = malloc(capacity * sizeof(token));
    if (tokens == NULL) {
        return NULL;
    }
    count = 0;
    int32_t i = 0;

    while (expr[i] != '\0') {
        if (count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, capacity * sizeof(token));
        }

        if (expr[i] == ' ' || expr[i] == '\t' || expr[i] == '\n') {
            i++;
            continue;
        }

        if (isdigit((unsigned char)expr[i])) {
            char buf[MAX_TOKEN_LEN];
            int32_t j = 0;
            while (isdigit((unsigned char)expr[i])) {
                buf[j++] = expr[i++];
            }
            buf[j] = '\0';

            tokens[count].value = malloc((strlen(buf) + 1) * sizeof(char));
            if (tokens[count].value == NULL) {
                free_tokens(tokens, count);
                return NULL;
            }
            strcpy(tokens[count].value, buf);
            tokens[count].type = OPERAND;
            (count)++;
            continue;
        }

        if (isalpha((unsigned char)expr[i])) {
            char buf[MAX_TOKEN_LEN];
            int32_t j = 0;
            while (isalpha((unsigned char)expr[i])) {
                buf[j++] = expr[i++];
            }
            buf[j] = '\0';

            tokens[count].value = malloc((strlen(buf) + 1) * sizeof(char));
            if (tokens[count].value == NULL) {
                free_tokens(tokens, count);
                return NULL;
            }
            strcpy(tokens[count].value, buf);
            tokens[count].type = OPERAND;
            (count)++;
            continue;
        }

        if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
            tokens[count].value = malloc(2 * sizeof(char));
            if (tokens[count].value == NULL) {
                free_tokens(tokens, count);
                return NULL;
            }
            strcpy(tokens[count].value, (char[]){expr[i], '\0'});
            tokens[count].type = OPERATOR;
            (count)++;
            i++;
            continue;
        }

        if (expr[i] == '(') {
            tokens[count].value = malloc(2 * sizeof(char));
            if (tokens[count].value == NULL) {
                free_tokens(tokens, count);
                return NULL;
            }
            strcpy(tokens[count].value, "(");
            tokens[count].type = OPEN_PARENT;
            (count)++;
            i++;
            continue;
        }

        if (expr[i] == ')') {
            tokens[count].value = malloc(2 * sizeof(char));
            if (tokens[count].value == NULL) {
                free_tokens(tokens, count);
                return NULL;
            }
            strcpy(tokens[count].value, ")");
            tokens[count].type = CLOSE_PARENT;
            (count)++;
            i++;
            continue;
        }

        i++;
    }

    return tokens;
}

int32_t priority(char* op) {
    if (strcmp(op, "(") == 0) {
        return 0;
    }
    if (strcmp(op, "+") == 0) {
        return 1;
    }
    if (strcmp(op, "-") == 0) {
        return 1;
    }
    if (strcmp(op, "*") == 0) {
        return 2;
    }
    if (strcmp(op, "/") == 0) {
        return 2;
    }

    return 0;
}

Node* construct_subtree(token* op_token, NodeStack* node_stack) {
    if (node_stack->size < 2) {
        return NULL;
    }

    Node* node = create_node(op_token);
    if (node == NULL) {
        return NULL;
    }
    
    Node* right = pop_node(node_stack);
    Node* left = pop_node(node_stack);

    node->left = left;
    node->right = right;
    return node;
}


Node* shunting_yard(token* tokens, size_t tokens_len) {
    NodeStack* node_stack = create_node_stack(INITIAL_CAPACITY);
    OpStack* op_stack = create_op_stack(INITIAL_CAPACITY);

    if (node_stack == NULL || op_stack == NULL) {
        free_node_stack(node_stack);
        free_op_stack(op_stack);
        return NULL;
    }

    for (size_t i = 0; i < tokens_len; ++i) {
        token* cur_token = &tokens[i];
        if (cur_token->type == OPERAND) {
            Node* node = create_node(cur_token);
            if (node == NULL) {
                free_node_stack(node_stack);
                free_op_stack(op_stack);
                return NULL;
            }

            int32_t status = push_node(node_stack, node);
            if (status == EXIT_FAILURE) {
                free(node);
                free_node_stack(node_stack);
                free_op_stack(op_stack);
                return NULL;
            }
        }

        if (cur_token->type == OPEN_PARENT) {
            int32_t status = push_op(op_stack, cur_token);
            if (status == EXIT_FAILURE) {
                free_node_stack(node_stack);
                free_op_stack(op_stack);
                return NULL;
            }
        }

        if (cur_token->type == OPERATOR) {
            while (op_stack->size > 0) {
                token* top = peek_op(op_stack);
                if (strcmp(top->value, "(") == 0) {
                    break;
                }
                if (priority(top->value) < priority(cur_token->value)) {
                    break;
                }
                token* op_token = pop_op(op_stack);

                Node* node = construct_subtree(op_token, node_stack);
                if (node == NULL) {
                    free_node_stack(node_stack);
                    free_op_stack(op_stack);
                    return NULL;
                }

                int32_t status = push_node(node_stack, node);
                if (status == EXIT_FAILURE) {
                    free_tree(node);
                    free_node_stack(node_stack);
                    free_op_stack(op_stack);
                    return NULL;
                }
            }

            int32_t status = push_op(op_stack, cur_token);
            if (status == EXIT_FAILURE) {
                free_node_stack(node_stack);
                free_op_stack(op_stack);
                return NULL;
            }
        }

        if (cur_token->type == CLOSE_PARENT) {
            while (op_stack->size > 0) {
                token* op_token = pop_op(op_stack);
                if (strcmp(op_token->value, "(") == 0) {
                    break;
                }

                Node* node = construct_subtree(op_token, node_stack);
                if (node == NULL) {
                    free_node_stack(node_stack);
                    free_op_stack(op_stack);
                    return NULL;
                }

                int32_t status = push_node(node_stack, node);
                if (status == EXIT_FAILURE) {
                    free_tree(node);
                    free_node_stack(node_stack);
                    free_op_stack(op_stack);
                    return NULL;
                }
            }
        }
        
    }

    while (op_stack->size > 0) {
        token* op_token = pop_op(op_stack);
        if (strcmp(op_token->value, "(") == 0) {
            free_node_stack(node_stack);
            free_op_stack(op_stack);
            return NULL;
        }

        Node* node = construct_subtree(op_token, node_stack);
        if (node == NULL) {
            free_node_stack(node_stack);
            free_op_stack(op_stack);
            return NULL;
        }

        int32_t status = push_node(node_stack, node);
        if (status == EXIT_FAILURE) {
            free_tree(node);
            free_node_stack(node_stack);
            free_op_stack(op_stack);
            return NULL;
        }
    }

    if (node_stack->size != 1) {
        free_node_stack(node_stack);
        free_op_stack(op_stack);
        return NULL;
    }

    Node* root = pop_node(node_stack);
    free_node_stack(node_stack);
    free_op_stack(op_stack);
    return root;
}

Node* simplify_div_by_one(Node* root) {
    if (root == NULL) {
        return NULL;
    }

    root->left = simplify_div_by_one(root->left);
    root->right = simplify_div_by_one(root->right);

    if (root->data != NULL &&
        root->data->type == OPERATOR &&
        strcmp(root->data->value, "/") == 0 &&
        root->left != NULL &&
        root->left->data != NULL &&
        root->right != NULL &&
        root->right->data != NULL &&
        root->right->data->type == OPERAND &&
        strcmp(root->right->data->value, "1") == 0) {

        Node* left = root->left;
        free_tree(root->right);
        free(root);
        return left;
    }

    return root;
}

void print_tree(Node* root) {
    if (root == NULL) {
        return;
    }

    int32_t is_op = root->data != NULL && root->data->type == OPERATOR;

    if (is_op) printf("(");
    print_tree(root->left);
    printf("%s", root->data->value);
    print_tree(root->right);
    if (is_op) printf(")");
}

void run_algorithm(char* input) {
    int32_t token_count = 0;
    token* tokenized_input = tokenize(input, token_count);
    if (tokenized_input == NULL) {
        return;
    }

    Node* tree = shunting_yard(tokenized_input, token_count);
    if (tree == NULL) {
        free_tokens(tokenized_input, token_count);
        return;
    }

    print_tree(tree);
    printf("\n");

    tree = simplify_div_by_one(tree);
    print_tree(tree);
    printf("\n");

    free_tree(tree);
    free_tokens(tokenized_input, token_count);
}

char* read_input(void) {
    size_t capacity = 64;
    size_t len = 0;
    char* buffer = malloc(capacity * sizeof(char));
    int c;

    if (buffer == NULL) {
        return NULL;
    }

    while ((c = getchar()) != EOF && c != '\n') {
        if (len + 1 >= capacity) {
            capacity *= 2;
            char* tmp = realloc(buffer, capacity * sizeof(char));
            if (tmp == NULL) {
                free(buffer);
                return NULL;
            }
            buffer = tmp;
        }

        buffer[len++] = (char)c;
    }

    buffer[len] = '\0';
    return buffer;
}

int main(void) {
    char* input = read_input();
    if (input == NULL) {
        printf("Input error\n");
        return 1;
    }

    run_algorithm(input);
    free(input);
    return 0;
}