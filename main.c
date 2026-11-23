#include <errno.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#include "argparse.h"
#include "query.h"
#include "notes.h"


static const char *DB_PATH = "./db.sqlite";

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

    sqlite3 *db_handle = open_notes(DB_PATH);

    if (db_handle == NULL) {
        perror("Couldn't open database!\n");
        exit(-1);
    }
    if (!validate_schema(db_handle)) {
        perror("Couldn't create database schema, failure on validation!");
        exit(-1);
    }

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
        query_result *result = list_notes(db_handle);
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
    return 0;
}
