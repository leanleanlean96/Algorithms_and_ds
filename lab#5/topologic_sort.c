#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



int32_t** create_incidence_matrix(size_t num_vertices, size_t num_edges) {
    int32_t **matrix = malloc(num_vertices * sizeof(int32_t*));
    if (matrix == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    for (size_t i = 0; i < num_vertices; ++i) {
        matrix[i] = NULL;
        if (num_edges == 0) {
            continue;
        }

        matrix[i] = calloc(num_edges, sizeof(int32_t));
        if (matrix[i] == NULL) {
            printf("Memory allocation failed\n");
            for (size_t j = 0; j < i; ++j) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }

    return matrix;
}

void free_incidence_matrix(int32_t **matrix, size_t num_vertices) {
    for (size_t i = 0; i < num_vertices; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

int dfs(int32_t** incidence_matrix, size_t* ans, size_t* pos, size_t* visited, size_t num_vertices, size_t num_edges, size_t vertex) {
    visited[vertex] = 1;

    for (size_t edge = 0; edge < num_edges; ++edge) {
        if (incidence_matrix[vertex][edge] == -1) {
            for (size_t to = 0; to < num_vertices; ++to) {
                if (incidence_matrix[to][edge] == 1) {
                    if (visited[to] == 1) {
                        return 0;
                    }

                    if (visited[to] == 0) {
                        if (dfs(incidence_matrix, ans, pos, visited,
                                 num_vertices, num_edges, to) == 0) {
                            return 0;
                        }
                    }
                }
            }
        }
    }

    visited[vertex] = 2;
    ans[*pos] = vertex;
    (*pos)++;

    return 1;
}

size_t* topological_sort(int32_t** incidence_matrix, size_t num_vertices, size_t num_edges) {
    if (num_vertices == 0) {
        return NULL;
    }

    size_t* visited = calloc(num_vertices, sizeof(size_t));
    size_t* ans = malloc(num_vertices * sizeof(size_t));

    if (visited == NULL || ans == NULL) {
        free(visited);
        free(ans);
        return NULL;
    }

    size_t pos = 0;

    for (size_t vertex = 0; vertex < num_vertices; ++vertex) {
        if (visited[vertex] == 0) {
            if (!dfs(incidence_matrix, ans, &pos, visited,
                     num_vertices, num_edges, vertex)) {
                free(visited);
                free(ans);
                return NULL;
            }
        }
    }

    size_t left = 0;
    size_t right = num_vertices - 1;

    while (left < right) {
        size_t temp = ans[left];
        ans[left] = ans[right];
        ans[right] = temp;
        left++;
        right--;
    }

    free(visited);
    return ans;
}

int main(void)
{
    size_t num_vertices;
    size_t num_edges;

    printf("Enter the number of vertices: ");
    if (scanf("%zu", &num_vertices) != 1) {
        printf("Invalid number of vertices\n");
        return 1;
    }
    if (num_vertices == 0) {
        printf("Number of vertices must be a positive integer\n");
        return 1;
    }

    printf("Enter the number of edges: ");
    if (scanf("%zu", &num_edges) != 1) {
        printf("Invalid number of edges\n");
        return 1;
    }

    int32_t** incidence_matrix = create_incidence_matrix(num_vertices, num_edges);
    if (incidence_matrix == NULL) {
        printf("Failed to create incidence matrix\n");
        return 1;
    }

    for (size_t edge = 0; edge < num_edges; ++edge) {
        size_t from;
        size_t to;

        printf("Enter edge %zu (from to): ", edge + 1);
        if (scanf("%zu %zu", &from, &to) != 2) {
            printf("Invalid edge input\n");
            free_incidence_matrix(incidence_matrix, num_vertices);
            return 1;
        }

        if (from == 0 || from > num_vertices || to == 0 || to > num_vertices) {
            printf("Vertices must be in range from 1 to %zu\n", num_vertices);

            free_incidence_matrix(incidence_matrix, num_vertices);

            return 1;
        }

        if (from == to) {
            printf("Can't sort a cycled graph\n");
            free_incidence_matrix(incidence_matrix, num_vertices);
            return 1;
        }

        incidence_matrix[from - 1][edge] = -1;
        incidence_matrix[to - 1][edge] = 1;
    }

    size_t* result = topological_sort(incidence_matrix, num_vertices, num_edges);
    if (result == NULL) {
        printf("Failed to sort\n");
        free_incidence_matrix(incidence_matrix, num_vertices);
        return 1;
    }

    printf("Topological order: ");
    for (size_t i = 0; i < num_vertices; ++i) {
        printf("%zu ", result[i] + 1);
    }
    printf("\n");

    free(result);
    free_incidence_matrix(incidence_matrix, num_vertices);

    return 0;
}
