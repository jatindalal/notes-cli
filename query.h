#ifndef QUERY_H
#define QUERY_H

#include <stdlib.h>

typedef struct query_row_t {
    char **col_values;
    char **col_names;
    size_t num_cols;
} query_row;

query_row *create_row(char **col_values, char **col_names, size_t num_cols);
void free_row(query_row *row);

typedef struct query_result_t {
    query_row **rows;
    size_t num_rows;
    size_t capacity;
} query_result;

query_result *create_result();
void push_back(query_result *result, query_row *row);
void pop_back(query_result *result);
void free_result(query_result *result);

#endif // QUERY_H
