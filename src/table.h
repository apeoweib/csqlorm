#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include "table_map.h"

enum ColumnType {
    S_TEXT,
    S_INT,
    S_DOUBLE
};

struct Column {
    char *name;
    enum ColumnType type;
    int is_unique;
    int is_primary_key;
    int is_not_null;
};


struct ForeignKey {
    struct Column *src_column;
    struct Table *dst_table;
    struct Column *dst_column;
};

struct Table {
    char *name;
    struct Column *columns;
    size_t num_columns;
    struct ForeignKey *foreign_keys;
    size_t num_foreign_keys;
};

void Table_create(const struct Table *table, char query[]);
void Table_drop(const struct Table *table, char query[]);
void Table_insert(char *data, struct TableMap *map, char query[]);

#endif
