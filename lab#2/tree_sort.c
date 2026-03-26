#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SUCCESS 0
#define OPERATION_FAILED 1


typedef struct {
    float key;
    float value;
} Node;

typedef struct {
    Node* data;
    size_t size;
    size_t capacity;
} DynamicArray;

int32_t create_array(DynamicArray* array) {
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
    return SUCCESS;
}

int32_t free_array(DynamicArray* array) {
    free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
    return SUCCESS;
}

int32_t reserve(DynamicArray* array, size_t new_capacity) {
    if (new_capacity <= array->capacity) {
        return SUCCESS;
    }

    Node *new_data = realloc(array->data, new_capacity * sizeof(Node));
    if (new_data == NULL) {
        return OPERATION_FAILED;
    }

    array->data = new_data;
    array->capacity = new_capacity;
    return SUCCESS;
}

int32_t append(DynamicArray* array, float key, float value) {
    if (array->size == array->capacity) {
        size_t new_capacity;
        if (array->capacity == 0) {
            new_capacity = 4;
        } else {
            new_capacity = array->capacity * 2;
        }
        if (reserve(array, new_capacity) != SUCCESS) {
            return OPERATION_FAILED;
        }
    }

    array->data[array->size].key = key;
    array->data[array->size].value = value;
    array->size++;
    return SUCCESS;
}

void swap_node(Node* first, Node* second) {
    Node temporary = *first;
    *first = *second;
    *second = temporary;
}

void heapify(DynamicArray* array, size_t heap_size, size_t root_index) {
    size_t largest_index = root_index;
    size_t left_child_index = 2 * root_index + 1;
    size_t right_child_index = 2 * root_index + 2;

    if (left_child_index < heap_size && array->data[left_child_index].key > array->data[largest_index].key) {
        largest_index = left_child_index;
    }

    if (right_child_index < heap_size && array->data[right_child_index].key > array->data[largest_index].key) {
        largest_index = right_child_index;
    }

    if (largest_index != root_index) {
        swap_node(&array->data[root_index], &array->data[largest_index]);
        heapify(array, heap_size, largest_index);
    }
}

int32_t heapsort(DynamicArray* array) {
    size_t heap_size = array->size;
    if (heap_size < 2) {
        return SUCCESS;
    }

    for (size_t i = heap_size / 2; i > 0; --i) {
        heapify(array, heap_size, i - 1);
    }

    for (size_t i = heap_size; i > 1; --i) {
        swap_node(&array->data[0], &array->data[i - 1]);
        heapify(array, i - 1, 0);
    }

    return SUCCESS;
}

int32_t print_array(DynamicArray* array) {
    for (size_t i = 0; i < array->size; i++) {
        printf("%f %f\n", array->data[i].key, array->data[i].value);
    }
    return SUCCESS;
}

int32_t binary_search(DynamicArray* array, size_t array_size, float key, float* output) {
    if (array_size == 0) {
        return OPERATION_FAILED;
    }

    size_t low = 0;
    size_t high = array_size - 1;
    
    while (low <= high) {
        size_t mid = (low + high) / 2;

        if (array->data[mid].key == key) {
            *output = array->data[mid].value;
            return SUCCESS;
        }

        if (array->data[mid].key > key) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    return OPERATION_FAILED;
}

int32_t read_from_file(DynamicArray* array,char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error reading file");
        return OPERATION_FAILED;
    }
 
    float key, value;
    while (fscanf(file, "%f %f", &key, &value) == 2) {
        if (append(array, key, value) != SUCCESS) {
            fclose(file);
            return OPERATION_FAILED;
        }
    }
 
    fclose(file);
    return SUCCESS;
}

void run_test(char *filename, char *description) {
    printf("%s", description);
    DynamicArray array;
    if (create_array(&array) != SUCCESS) {
        return;
    }

    if (read_from_file(&array, filename) != SUCCESS) {
        free_array(&array);
        return;
    }

    printf("\nBefore sorting:\n");
    print_array(&array);

    if (heapsort(&array) != SUCCESS) {
        free_array(&array);
        return;
    }

    printf("\nAfter sorting:\n");
    print_array(&array);

    printf("\nBinary search:\n");

    for (size_t i = 0; i < array.size; i++) {
        float found_value;
        if (binary_search(&array, array.size, array.data[i].key, &found_value) == SUCCESS) {
            printf("key: %f value: %f\n", array.data[i].key, found_value);
        } else {
            printf("key: %f is not presented in the array\n", array.data[i].key);
        }
    }

    free_array(&array);
    printf("\n");
}

int32_t main() {
    run_test("sorted.txt",   "Sorted array usecase");
    run_test("reversed.txt", "Reverse sorted array usecase");
    run_test("random.txt",   "Unsorted array usecase");
    return SUCCESS;
}
