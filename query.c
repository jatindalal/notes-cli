#include "query.h"
#include <stdio.h>
#include <string.h>

query_row *create_row(char **col_values, char **col_names, size_t num_cols)
{
    query_row *new_row = (query_row *)malloc(sizeof(query_row));
    new_row->num_cols = num_cols;
    new_row->col_values = (char **)malloc(num_cols * sizeof(char *));
    new_row->col_names = (char **)malloc(num_cols * sizeof(char *));

    for (int i = 0; i < num_cols; ++i) {
        // creating a new buffer to have copy of an existing string needs to take in account
        // the space for a null character
        new_row->col_values[i] = (char *)malloc((strlen(col_values[i]) + 1) * sizeof(char));
        strcpy(new_row->col_values[i], col_values[i]);
        new_row->col_names[i] = (char *)malloc((strlen(col_names[i]) + 1) * sizeof(char));
        strcpy(new_row->col_names[i], col_names[i]);
    }
    return new_row;
}

void free_row(query_row *row)
{
    for (int i = 0; i < row->num_cols; ++i) {
        free(row->col_values[i]);
        free(row->col_names[i]);
    }
    free(row->col_values);
    free(row->col_names);
    free(row);
}

query_result *create_result()
{
    query_result *result = (query_result *)malloc(sizeof(query_result));
    result->rows = NULL;
    result->num_rows = 0;
    result->capacity = 0;
    return result;
}

void push_back(query_result *result, query_row *row)
{
    if (result->rows == NULL) {
        result->rows = (query_row **)malloc(sizeof(query_row *));
        result->capacity = 1;
    } else if (result->num_rows == result->capacity) {
        query_row **new_rows = (query_row **)realloc(
            result->rows,
            result->capacity * 2 * sizeof(query_row *));
        if (new_rows == NULL) {
            perror("Resize error for vector!");
            exit(-1);
        }
        result->rows = new_rows;
        result->capacity *= 2;
    }
    result->rows[result->num_rows] = row;
    result->num_rows++;
}

void pop_back(query_result *result)
{
    if (result->num_rows > 0) {
        result->num_rows--;
    }
}

void free_result(query_result *result)
{
    for (int i = 0; i < result->num_rows; ++i) {
        free_row(result->rows[i]);
    }
    free(result->rows);
    free(result);
}
