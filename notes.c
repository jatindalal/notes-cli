#include "notes.h"

sqlite3 *open_notes(const char *db_path)
{
    sqlite3 *db_handle;
    sqlite3_open_v2(db_path, &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    return db_handle;
}

int _query_callback(void *data,
    int num_cols,
    char **col_values,
    char **col_names)
{
    if (data != NULL) {
        query_result *result = (query_result *)data;
        query_row *row_copy = create_row(col_values, col_names, num_cols);
        push_back(result, row_copy);
    }
    return 0;
}

void validate_schema(sqlite3 *db_handle)
{
    char *error_msg;
    int exit_code = sqlite3_exec(db_handle, SCHEMA_QUERY, _query_callback, NULL,
        &error_msg);
    if (exit_code != SQLITE_OK) {
        perror("Schema validation failed!  ");
        perror(error_msg);
        exit(-1);
    }
}

void add_note(sqlite3 *db_handle, const char *note_text)
{
    // TODO: length provided in snprintf is not optimal
    const char *insert_query_format_str = "INSERT INTO note (text) VALUES(\"%s\")";
    char *error_msg;
    char query_buffer[10240];
    snprintf(query_buffer, strlen(insert_query_format_str) + strlen(note_text),
        insert_query_format_str, note_text);
    if (sqlite3_exec(db_handle, query_buffer, _query_callback, NULL, &error_msg)
        != SQLITE_OK) {
        perror("error during insertion: ");
        perror(error_msg);
        exit(-1);
    }
}

void delete_note(sqlite3 *db_handle, int note_id)
{
    // TODO: length provided in snprintf is not optimal
    // TODO: delete silently fails if note_id is not present
    const char *delete_query_format_str = "DELETE from note where id = \"%d\"";
    char query_buffer[40];
    char *error_msg;
    snprintf(query_buffer, 40, delete_query_format_str, note_id);
    if (sqlite3_exec(db_handle, query_buffer, _query_callback, NULL, &error_msg)
        != SQLITE_OK) {
        perror("error during deletion: ");
        perror(error_msg);
        exit(-1);
    }
}

void list_notes(sqlite3 *db_handle)
{
    char *error_msg;
    const char *list_query = "SELECT * FROM note;";
    query_result *result = create_result();
    if (sqlite3_exec(db_handle, list_query, _query_callback, result, &error_msg)
        != SQLITE_OK) {
        perror("error while listing notes: ");
        perror(error_msg);
        exit(-1);
    }

    for (int i = 0; i < result->num_rows; ++i) {
        for (int j = 0; j < result->rows[i]->num_cols; ++j) {
            printf("%s: %s, ",
                result->rows[i]->col_names[j],
                result->rows[i]->col_values[j]);
        }
        printf("\n");
    }
    free_result(result);
}
