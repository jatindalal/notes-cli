#include <errno.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argparse.h"

const char *SCHEMA_QUERY = "                                \
CREATE TABLE IF NOT EXISTS \"note\" (                       \
	\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,      \
	\"text\" VARCHAR,                                       \
	\"day\" TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL    \
);";

const char *DB_PATH = "./db.sqlite";

int query_callback(void *data, int argc, char **argv, char **col_name)
{
    for (int i = 0; i < argc; ++i) {
        printf("%s = %s, ", col_name[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void validate_schema(sqlite3 *db_handle)
{
    char *error_msg;
    int exit_code = sqlite3_exec(db_handle, SCHEMA_QUERY, query_callback, NULL, &error_msg);
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
    snprintf(query_buffer, strlen(insert_query_format_str) + strlen(note_text), insert_query_format_str, note_text);
    if (sqlite3_exec(db_handle, query_buffer, query_callback, NULL, &error_msg) != SQLITE_OK) {
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
    if (sqlite3_exec(db_handle, query_buffer, query_callback, NULL, &error_msg) != SQLITE_OK) {
        perror("error during deletion: ");
        perror(error_msg);
        exit(-1);
    }
}

void list_notes(sqlite3 *db_handle)
{
    char *error_msg;
    const char *list_query = "SELECT * FROM note;";
    if (sqlite3_exec(db_handle, list_query, query_callback, NULL, &error_msg) != SQLITE_OK) {
        perror("error while listing notes: ");
        perror(error_msg);
        exit(-1);
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

    if (argc == 1 || (!add_item && !del_item && !list_item)) {
        argparse_usage(&argparse);
        exit(-1);
    }
    if (argc != 0) {
        printf("argc: %d\n", argc);
        int i;
        for (i = 0; i < argc; i++) {
            printf("argv[%d]: %s\n", i, *(argv + i));
        }
    }

    sqlite3 *db_handle;
    sqlite3_open_v2(DB_PATH, &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    if (db_handle == NULL) {
        perror("Couldn't open database!\n");
        exit(-1);
    }
    validate_schema(db_handle);

    if (add_item != NULL) {
        add_note(db_handle, add_item);
    }
    if (del_item != NULL) {
        char *end_ptr = NULL;
        int id = strtol(del_item, &end_ptr, 10);
        if (end_ptr) {
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
