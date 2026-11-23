#include "notes.h"
#include <sqlite3.h>

sqlite3 *open_notes(const char *db_path)
{
    sqlite3 *db_handle;
    sqlite3_open_v2(db_path, &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    return db_handle;
}

bool close_notes(sqlite3 *db_handle)
{
    if (sqlite3_close_v2(db_handle) == SQLITE_OK) {
        db_handle = NULL;
        return true;
    }
    return false;
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

bool validate_schema(sqlite3 *db_handle)
{
    char *error_msg;
    return sqlite3_exec(db_handle, SCHEMA_QUERY,
                        _query_callback, NULL, &error_msg) == SQLITE_OK;
}

bool add_note(sqlite3 *db_handle, const char *note_text)
{
    // TODO: length provided in snprintf is not optimal
    const char *insert_query_format_str = "INSERT INTO note (text) VALUES(\"%s\")";
    char *error_msg;
    char query_buffer[10240];
    snprintf(query_buffer, strlen(insert_query_format_str) + strlen(note_text),
        insert_query_format_str, note_text);
    return sqlite3_exec(db_handle, query_buffer, _query_callback, NULL, &error_msg)
        == SQLITE_OK;
}

bool delete_note(sqlite3 *db_handle, int note_id)
{
    // TODO: length provided in snprintf is not optimal
    // TODO: delete silently fails if note_id is not present
    const char *delete_query_format_str = "DELETE from note where id = \"%d\"";
    char query_buffer[40];
    char *error_msg;
    snprintf(query_buffer, 40, delete_query_format_str, note_id);
    return sqlite3_exec(db_handle, query_buffer, _query_callback, NULL, &error_msg)
        == SQLITE_OK;
}

query_result *list_notes(sqlite3 *db_handle)
{
    char *error_msg;
    const char *list_query = "SELECT * FROM note;";
    query_result *result = create_result();
    if (sqlite3_exec(db_handle, list_query, _query_callback, result, &error_msg) != SQLITE_OK) {
        free_result(result);
        return NULL;
    }
    return result;
}
