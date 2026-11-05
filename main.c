#include <errno.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"
#include "query.h"

const char *SCHEMA_QUERY = "                                \
CREATE TABLE IF NOT EXISTS \"note\" (                       \
	\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,      \
	\"text\" VARCHAR,                                       \
	\"day\" TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL    \
);";

static const char *DB_PATH = "./db.sqlite";

int query_callback(void *data,
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
    int exit_code = sqlite3_exec(db_handle, SCHEMA_QUERY, query_callback, NULL,
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
    if (sqlite3_exec(db_handle, query_buffer, query_callback, NULL, &error_msg)
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
    if (sqlite3_exec(db_handle, query_buffer, query_callback, NULL, &error_msg)
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
    if (sqlite3_exec(db_handle, list_query, query_callback, result, &error_msg)
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
}

static const char *const usages[] = {
    "basic [options] [[--] args]",
    "basic [options]",
    NULL,
};

int main(int argc, const char **argv)
{
    const char *add_item = NULL;
    const char *del_item = NULL;
    int list_item = 0;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('a', "add", &add_item, NULL, 0, 0),
        OPT_STRING('d', "delete", &del_item, NULL, 0, 0),
        OPT_BOOLEAN('l', "list", &list_item, NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nA todo app", "");
    argc = argparse_parse(&argparse, argc, argv);

    if (!add_item && !del_item && !list_item) {
        argparse_usage(&argparse);
        exit(-1);
    }

    sqlite3 *db_handle;
    sqlite3_open_v2(DB_PATH, &db_handle,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    if (db_handle == NULL) {
        perror("Couldn't open database!\n");
        exit(-1);
    }
    validate_schema(db_handle);

    if (add_item != NULL) {
        add_note(db_handle, add_item);
    }
    if (del_item != NULL) {
        char *end_ptr;
        int id = strtol(del_item, &end_ptr, 10);
        if (*end_ptr != '\0') {
            printf("Invalid delete item id \"%s\"\n", del_item);
            exit(-1);
        }
        delete_note(db_handle, id);
    }
    if (list_item != 0) {
        list_notes(db_handle);
    }
    return 0;
}
