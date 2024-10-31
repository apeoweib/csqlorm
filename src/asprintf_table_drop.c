#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/table.h"


int asprintf_table_drop(char **query, const struct Table *table) {
    const char drop_table[] = " drop table if exists %s;\n";
    const size_t drop_table_len = sizeof(drop_table);

    size_t size = drop_table_len + strlen(table->name);

    *query = calloc(size+1, sizeof(char));
    if(!(*query)) {
        return -1;
    }
    return sprintf(*query, drop_table, table->name);
}


