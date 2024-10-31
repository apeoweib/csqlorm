#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/table.h"

static const char sql_int_type[] = "integer";
static const char sql_double_type[] = "double";
static const char sql_text_type[] = "text";

static const char *type_to_str(enum ColumnType type) {
    switch(type) {
        case S_INT:
            return sql_int_type;
        case S_TEXT:
            return sql_text_type;
        case S_DOUBLE:
            return sql_double_type;
    }
}

int asprintf_table_create(char **query, const struct Table *table) {
    const char create_table[] = " create table %s (";
    const size_t create_table_len = sizeof(create_table);
    const char create_table_end[] = ");\n";
    const size_t create_table_end_len = sizeof(create_table_end);
    const char create_column[] = "%s %s";
    const size_t create_column_len = sizeof(create_column);
    const char column_separator[] = ", ";
    const size_t column_separator_len = sizeof(column_separator);
    const char create_foreign_key[] = "foreign key(%s) references %s(%s)";
    const size_t create_foreign_key_len = strlen(create_foreign_key);
    const char is_unique[] = " unique";
    const size_t is_unique_len = strlen(is_unique);
    const char is_primary_key[] = " primary key";
    const size_t is_primary_key_len = strlen(is_primary_key);
    const char is_not_null[] = " not null";
    const size_t is_not_null_len = strlen(is_not_null);

    size_t foreign_keys_size = 0;
    for(size_t i = 0; i < table->num_foreign_keys; i++) {
        struct ForeignKey *f = &table->foreign_keys[i];
        foreign_keys_size += create_foreign_key_len +
            strlen(f->src_column->name) +
            strlen(f->dst_table->name) +
            strlen(f->dst_column->name);
    }
    size_t columns_size = 0;
    for(size_t i = 0; i < table->num_columns; i++) {
        struct Column *c = &table->columns[i];
        columns_size += create_column_len + strlen(c->name)
            + strlen(type_to_str(c->type));
        if(c->constraints & CC_PRIMARY_KEY) {
            columns_size += is_primary_key_len;
        } else {
            if(c->constraints & CC_PRIMARY_KEY) {
                columns_size += is_unique_len;
            } else if(c->constraints & CC_NOT_NULL) {
                columns_size += is_not_null_len;
            }
        }
        if(i < table->num_columns - 1 || table->num_foreign_keys > 0) {
            columns_size += column_separator_len;
        }
    }
    size_t size = create_table_len + strlen(table->name) + foreign_keys_size +
        columns_size + create_table_end_len;

    *query = calloc(size+1, sizeof(char));
    if(!(*query)) {
        return -1;
    }
    if(sprintf(*query, create_table, table->name) < 0) 
        return -1;
    for(size_t i = 0; i < table->num_columns; i++) {
        char column_def[512];
        struct Column *c = &table->columns[i];
        if(sprintf(column_def, create_column, c->name,
                type_to_str(c->type)) < 0)
                return -1;
        if(c->constraints & CC_PRIMARY_KEY) {
            strcat(column_def, is_primary_key);
        } else {
            if(c->constraints & CC_UNIQUE) {
                strcat(column_def, is_unique);
            } else if(c->constraints & CC_NOT_NULL) {
                strcat(column_def, is_not_null);
            }
        }
        if(i < table->num_columns - 1 || table->num_foreign_keys > 0) {
            strcat(column_def, column_separator);
        }
        strcat(*query, column_def);
    }
    for(size_t i = 0; i < table->num_foreign_keys; i++) {
        char foreign_key_def[512];
        struct ForeignKey *f = &table->foreign_keys[i];
        if(sprintf(foreign_key_def, create_foreign_key,
                f->src_column->name,
                f->dst_table->name,
                f->dst_column->name) < 0)
            return -1;
        strcat(*query, foreign_key_def);
    }
    strcat(*query, create_table_end);
    return size;
}
