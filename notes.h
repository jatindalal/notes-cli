#ifndef NOTES_H
#define NOTES_H

#include <errno.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"

static const char *SCHEMA_QUERY = "                         \
CREATE TABLE IF NOT EXISTS \"note\" (                       \
	\"id\" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,      \
	\"text\" VARCHAR,                                       \
	\"day\" TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL    \
);";


int _query_callback(void *data, int num_cols, char **col_values, char **col_names);

sqlite3 *open_notes(const char *db_path);
void validate_schema(sqlite3 *db_handle);
void add_note(sqlite3 *db_handle, const char *note_text);
void delete_note(sqlite3 *db_handle, int note_id);
void list_notes(sqlite3 *db_handle);

#endif // NOTES_H
