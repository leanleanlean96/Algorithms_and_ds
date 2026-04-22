#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    BTREE_T      = 2,
    MIN_KEYS     = 1,
    MAX_KEYS     = 3,
    MAX_CHILD    = 4,
    MAX_KEY_LEN  = 6,
    KEY_BUF_SIZE = 7,
    LINE_BUF     = 256
};

typedef enum {
    BT_OK = 0,
    BT_ALREADY_EXISTS,
    BT_NOT_FOUND
} Status;

typedef struct {
    char   key[KEY_BUF_SIZE];
    double value;
} Entry;

typedef struct Node {
    int           n;
    bool          is_leaf;
    Entry         entries [MAX_KEYS + 1];
    struct Node*  children[MAX_CHILD + 1];
} Node;

typedef struct {
    Node* root;
} BTree;


void clear_entry(Entry* e) {
    e->key[0] = '\0';
    e->value = 0.0;
}

void copy_key(char* dst, char* src) {
    int i = 0;
    while (i < MAX_KEY_LEN && src[i] != '\0') {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}

Node* create_node(bool is_leaf) {
    Node* x = (Node*)malloc(sizeof(Node));
    if (!x) {
        printf("Error allocating memory\n");
        return NULL;
    }
    x->n = 0;
    x->is_leaf = is_leaf;
    for (int i = 0; i <= MAX_CHILD; ++i) {
        x->children[i] = NULL;
    }
    for (int i = 0; i <= MAX_KEYS; ++i) {
        clear_entry(&x->entries[i]);
    }
    return x;
}

void free_node(Node* x) {
    if (!x) {
        return;
    }
    if (!x->is_leaf) {
        for (int i = 0; i <= x->n; ++i) {
            free_node(x->children[i]);
        }
    }
    free(x);
}

int find_node_index(Node* x, char* key) {
    int i = 0;
    while (i < x->n && strcmp(x->entries[i].key, key) < 0) {
        ++i;
    }
    return i;
}


Status node_search(Node* x, char* key, double* out) {
    if (!x) {
        return BT_NOT_FOUND;
    }
    int i = find_node_index(x, key);
    if (i < x->n && strcmp(x->entries[i].key, key) == 0) {
        if (out) {
            *out = x->entries[i].value;
        }
        return BT_OK;
    }
    if (x->is_leaf) {
        return BT_NOT_FOUND;
    }
    return node_search(x->children[i], key, out);
}

Status btree_search(BTree* tree, char* key, double* out_value) {
    if (!tree || !tree->root) {
        return BT_NOT_FOUND;
    }
    return node_search(tree->root, key, out_value);
}

void node_print(Node* x, int depth, FILE* out) {
    if (!x) {
        return;
    }
    if (!x->is_leaf) {
        node_print(x->children[x->n], depth + 1, out);
    }

    for (int i = 0; i < depth; ++i) {
        fprintf(out, "    ");
    }

    fprintf(out, "[");
    for (int i = 0; i < x->n; ++i) {
        if (i > 0) {
            fprintf(out, " | ");
        }
        fprintf(out, "%s=%g", x->entries[i].key, x->entries[i].value);
    }
    fprintf(out, "]\n");

    if (!x->is_leaf) {
        for (int i = x->n - 1; i >= 0; --i) {
            node_print(x->children[i], depth + 1, out);
        }
    }
}

void btree_print(BTree* tree, FILE* out) {
    if (!tree || !tree->root) {
        fprintf(out, "The tree is empty\n");
        return;
    }
    node_print(tree->root, 0, out);
}

void split_child(Node* parent, int i) {
    Node* y = parent->children[i];
    Node* z = create_node(y->is_leaf);

    z->n = 1;
    z->entries[0] = y->entries[2];

    if (!y->is_leaf) {
        z->children[0] = y->children[2];
        z->children[1] = y->children[3];
        y->children[2] = NULL;
        y->children[3] = NULL;
    }

    Entry middle = y->entries[1];
    y->n = 1;

    clear_entry(&y->entries[1]);
    clear_entry(&y->entries[2]);

    for (int j = parent->n; j > i; --j) {
        parent->entries[j] = parent->entries[j - 1];
    }
    for (int j = parent->n + 1; j > i + 1; --j) {
        parent->children[j] = parent->children[j - 1];
    }

    parent->entries[i] = middle;
    parent->children[i + 1] = z;
    parent->n += 1;
}

void insert_nonfull(Node* x, Entry* e) {
    int i = find_node_index(x, e->key);

    if (x->is_leaf) {
        for (int j = x->n; j > i; --j) {
            x->entries[j] = x->entries[j - 1];
        }
        x->entries[i] = *e;
        x->n += 1;
        return;
    }

    if (x->children[i]->n == MAX_KEYS) {
        split_child(x, i);
        if (strcmp(e->key, x->entries[i].key) > 0) {
            ++i;
        }
    }
    insert_nonfull(x->children[i], e);
}

Status btree_insert(BTree* tree, char* key, double value) {
    if (btree_search(tree, key, NULL) == BT_OK) {
        return BT_ALREADY_EXISTS;
    }

    Entry e;
    copy_key(e.key, key);
    e.value = value;

    if (!tree->root) {
        tree->root = create_node(true);
        tree->root->entries[0] = e;
        tree->root->n = 1;
        return BT_OK;
    }

    Node* r = tree->root;
    if (r->n == MAX_KEYS) {
        Node* s = create_node(false);
        s->children[0] = r;
        tree->root = s;
        split_child(s, 0);
        insert_nonfull(s, &e);
    } else {
        insert_nonfull(r, &e);
    }
    return BT_OK;
}

void merge_children(Node* x, int i) {
    Node* left  = x->children[i];
    Node* right = x->children[i + 1];

    left->entries[left->n] = x->entries[i];

    for (int j = 0; j < right->n; ++j) {
        left->entries[left->n + 1 + j] = right->entries[j];
    }

    if (!left->is_leaf) {
        for (int j = 0; j <= right->n; ++j) {
            left->children[left->n + 1 + j] = right->children[j];
        }
    }

    left->n += 1 + right->n;

    for (int j = i; j < x->n - 1; ++j) {
        x->entries[j] = x->entries[j + 1];
    }
    for (int j = i + 1; j < x->n; ++j) {
        x->children[j] = x->children[j + 1];
    }
    x->n -= 1;
    x->children[x->n + 1] = NULL;
    clear_entry(&x->entries[x->n]);

    free(right);
}

void borrow_from_left(Node* x, int i) {
    Node* child = x->children[i];
    Node* left  = x->children[i - 1];

    for (int j = child->n; j > 0; --j) {
        child->entries[j] = child->entries[j - 1];
    }
    if (!child->is_leaf) {
        for (int j = child->n + 1; j > 0; --j) {
            child->children[j] = child->children[j - 1];
        }
    }

    child->entries[0] = x->entries[i - 1];

    if (!child->is_leaf) {
        child->children[0] = left->children[left->n];
        left->children[left->n] = NULL;
    }

    x->entries[i - 1] = left->entries[left->n - 1];

    child->n += 1;
    left->n  -= 1;
    clear_entry(&left->entries[left->n]);
}

void borrow_from_right(Node* x, int i) {
    Node* child = x->children[i];
    Node* right = x->children[i + 1];

    child->entries[child->n] = x->entries[i];

    if (!child->is_leaf) {
        child->children[child->n + 1] = right->children[0];
    }

    x->entries[i] = right->entries[0];

    for (int j = 0; j < right->n - 1; ++j) {
        right->entries[j] = right->entries[j + 1];
    }
    if (!right->is_leaf) {
        for (int j = 0; j < right->n; ++j) {
            right->children[j] = right->children[j + 1];
        }
        right->children[right->n] = NULL;
    }

    child->n += 1;
    right->n -= 1;
    clear_entry(&right->entries[right->n]);
}

void check_child(Node* x, int i) {
    if (x->children[i]->n >= BTREE_T) {
        return;
    }

    if (i > 0 && x->children[i - 1]->n >= BTREE_T) {
        borrow_from_left(x, i);
        return;
    }
    if (i < x->n && x->children[i + 1]->n >= BTREE_T) {
        borrow_from_right(x, i);
        return;
    }

    if (i < x->n) {
        merge_children(x, i);
    } else {
        merge_children(x, i - 1);
    }
}

Entry find_max_entry(Node* x) {
    while (!x->is_leaf) {
        x = x->children[x->n];
    }
    return x->entries[x->n - 1];
}

Entry find_min_entry(Node* x) {
    while (!x->is_leaf) {
        x = x->children[0];
    }
    return x->entries[0];
}

Status node_remove(Node* x, char* key) {
    int i = find_node_index(x, key);
    bool found_here =
        (i < x->n) && (strcmp(x->entries[i].key, key) == 0);

    if (found_here) {
        if (x->is_leaf) {
            for (int j = i; j < x->n - 1; ++j) {
                x->entries[j] = x->entries[j + 1];
            }
            x->n -= 1;
            clear_entry(&x->entries[x->n]);
            return BT_OK;
        }

        Node* y = x->children[i];
        Node* z = x->children[i + 1];

        if (y->n >= BTREE_T) {
            Entry pred = find_max_entry(y);
            x->entries[i] = pred;
            return node_remove(y, pred.key);
        } else if (z->n >= BTREE_T) {
            Entry succ = find_min_entry(z);
            x->entries[i] = succ;
            return node_remove(z, succ.key);
        } else {
            merge_children(x, i);
            return node_remove(y, key);
        }
    }

    if (x->is_leaf) {
        return BT_NOT_FOUND;
    }

    bool can_merge = false;
    if (x->children[i]->n < BTREE_T) {
        bool can_borrow_left  = (i > 0) &&
                                (x->children[i - 1]->n >= BTREE_T);
        bool can_borrow_right = (i < x->n) &&
                                (x->children[i + 1]->n >= BTREE_T);
        if (!can_borrow_left && !can_borrow_right && i == x->n) {
            can_merge = true;
        }
        check_child(x, i);
    }

    if (can_merge) {
        return node_remove(x->children[i - 1], key);
    }
    return node_remove(x->children[i], key);
}

Status btree_remove(BTree* tree, char* key) {
    if (!tree || !tree->root) {
        return BT_NOT_FOUND;
    }

    Status st = node_remove(tree->root, key);
    if (st != BT_OK) {
        return st;
    }

    if (tree->root->n == 0) {
        Node* old = tree->root;
        if (old->is_leaf) {
            tree->root = NULL;
        } else {
            tree->root = old->children[0];
        }
        free(old);
    }
    return BT_OK;
}

void btree_init(BTree* tree) {
    tree->root = NULL;
}

void btree_free(BTree* tree) {
    free_node(tree->root);
    tree->root = NULL;
}

bool is_valid_key(char* s) {
    if (!s || !*s) {
        return false;
    }
    size_t len = strlen(s);
    if (len > MAX_KEY_LEN) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            return false;
        }
    }
    return true;
}

void process_line(BTree* tree, char* line, char* original,
                         FILE* out) {
    fprintf(out, "> %s\n", original);

    char* op_str = strtok(line, " \t\r\n");
    if (!op_str) {
        fprintf(out, "  The input is empty\n\n");
        return;
    }
    int op = atoi(op_str);

    if (op == 1) {
        char* key = strtok(NULL, " \t\r\n");
        char* val = strtok(NULL, " \t\r\n");
        if (!key || !val) {
            fprintf(out, "  '1 <key> <value>' is expected\n\n");
            return;
        }
        if (!is_valid_key(key)) {
            fprintf(out, "  Invalid key '%s'\n\n", key);
            return;
        }
        double v = strtod(val, NULL);
        Status st = btree_insert(tree, key, v);
        if (st == BT_OK) {
            fprintf(out, "  inserted: %s = %g\n\n", key, v);
        } else {
            fprintf(out, "  key '%s' already exists\n\n", key);
        }
    } else if (op == 2) {
        char* key = strtok(NULL, " \t\r\n");
        if (!key || !is_valid_key(key)) {
            fprintf(out, "  '2 <key>' is expected\n\n");
            return;
        }
        Status st = btree_remove(tree, key);
        if (st == BT_OK) {
            fprintf(out, "  removed: %s\n\n", key);
        } else {
            fprintf(out, "  key '%s' not found\n\n", key);
        }
    } else if (op == 3) {
        fprintf(out, "  tree:\n");
        btree_print(tree, out);
        fprintf(out, "\n");
    } else if (op == 4) {
        char* key = strtok(NULL, " \t\r\n");
        if (!key || !is_valid_key(key)) {
            fprintf(out, "  '4 <key>' is expected\n\n");
            return;
        }
        double v = 0.0;
        Status st = btree_search(tree, key, &v);
        if (st == BT_OK) {
            fprintf(out, "  found: %s = %g\n\n", key, v);
        } else {
            fprintf(out, "  key '%s' not found\n\n", key);
        }
    } else {
        fprintf(out, "Unknown operation '%s'\n\n", op_str);
    }
}

int main(void) {
    FILE* in = fopen("input.txt", "r");
    if (!in) {
        printf("operation failed\n");
        return EXIT_FAILURE;
    }
    FILE* out = fopen("output.txt", "w");
    if (!out) {
        printf("operation failed\n");
        fclose(in);
        return EXIT_FAILURE;
    }

    BTree tree;
    btree_init(&tree);

    char buf[LINE_BUF];
    char original[LINE_BUF];

    while (fgets(buf, sizeof(buf), in)) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[--len] = '\0';
        }
        if (len == 0) {
            continue;
        }

        for (size_t k = 0; k <= len; ++k) {
            original[k] = buf[k];
        }

        process_line(&tree, buf, original, out);
    }

    btree_free(&tree);
    fclose(in);
    fclose(out);
    return EXIT_SUCCESS;
}
