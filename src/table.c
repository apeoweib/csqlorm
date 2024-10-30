#include <stdio.h>
#include <string.h>
#include "../include/table.h"

const char sql_int_type[] = "integer";
const char sql_double_type[] = "double";
const char sql_text_type[] = "text";

const char *type_to_str(enum ColumnType type) {
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

// use the Table struct to create a table in the sqlite database
int snprintf_table_create(char * restrict query, size_t size, const struct Table *table) {
    char *buf;
    int result = asprintf_table_create(&buf, table);

    // if buf is set to null, or result is less than zero, an error has occurred.
    // If buff is not null, but the value of result still indicates an
    // error, we'll need to return it
    if(!buf)
        return -1;
    if(result < 0) {
        free(buf);
        return -1;
    }

    // table creation succeeded.  We only want to copy the string
    result = strlen(buf); 


    // if the size argument is zero, the caller is just interested in
    // the necessary size of the buffer they must provide.
    if(size > 0) {
        if((long)size > result) {
            memcpy(query, buf, result+1);
        } else {
            // query is too small, we have to truncate buf and
            // end with a string terminator
            memcpy(query, buf, size - 1);
            query[size - 1] = 0;
        }
    }
    free(buf);
    return result;
}

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

int snprintf_table_drop(char * restrict query, size_t size, const struct Table *table) {

    char *buf;
    int result = asprintf_table_drop(&buf, table);

    // if buf is set to null, or result is less than zero, an error has occurred.
    // If buff is not null, but the value of result still indicates an
    // error, we'll need to return it
    if(!buf)
        return -1;
    if(result < 0) {
        free(buf);
        return -1;
    }

    result = strlen(buf); 

    // if the size argument is zero, the caller is just interested in
    // the necessary size of the buffer they must provide.
    if(size > 0) {
        if((long)size > result) {
            memcpy(query, buf, result+1);
        } else {
            // query is too small, we have to truncate buf and
            // end with a string terminator
            memcpy(query, buf, size - 1);
            query[size - 1] = 0;
        }
    }
    free(buf);
    return result;
}

int count_column_maps(struct FieldMap field_maps[],
        size_t num_field_maps) {
    size_t count = 0;
    for(size_t i = 0; i < num_field_maps; i++) {
        if(field_maps[i].type == FMT_COLUMN) {
            count++;
        }
    }
    return count;
}

int snprintf_table_insert(char * restrict query, size_t size,
        const char * restrict data, const struct TableMap *map) {
    (void)size;
    const char insert[] = " insert into %s (%s) values (";
    char column_names[512] = {0};
    size_t num_column_maps = count_column_maps(map->field_maps, map->num_field_maps);;
    size_t column_maps_added_so_far = 0;
    for(size_t i = 0; i < map->num_field_maps; i++) {
        struct FieldMap *field = &map->field_maps[i];
        switch(field->type) {
            case FMT_ARRAY:
                break;
            case FMT_COLUMN:
                {
                    strcat(column_names, field->column.column->name);
                    column_maps_added_so_far++;
                }
                break;
        }
        if(column_maps_added_so_far < num_column_maps)
            strcat(column_names, ", ");
    }
    sprintf(query, insert, map->table->name, column_names);
    column_maps_added_so_far = 0;
    for(size_t i = 0; i < map->num_field_maps; i++) {
        struct FieldMap *field = &map->field_maps[i];
        switch(field->type) {
            case FMT_ARRAY:
                // we skip doing anything for an array because we first
                // need to generate the sql for inserting this objects'
                // row.  Then we can call the scopy_identity() function
                // in the sql statement to retrieve the id of the newly
                // insertd row, and use this to generate the sql for
                // inserting into the table specified in the array field map.
                break;
            case FMT_COLUMN:
                {
                    column_maps_added_so_far++;
                    char column_query[512];
                    switch(field->column.column->type) {
                        case S_TEXT:
                            {
                                char *val = *(char **)(data + field->column.offset);
                                sprintf(column_query, "'%s'", val);
                            }
                            break;
                        case S_INT:
                            {
                                int val = *(int *)(data + field->column.offset);
                                sprintf(column_query, "%d", val);
                            }
                            break;
                        case S_DOUBLE:
                            {
                                float val = *(float *)(data + field->column.offset);
                                sprintf(column_query, "%0.3f", val);
                            }
                            break;
                    }
                    if(column_maps_added_so_far < num_column_maps) {
                        strcat(column_query, ", ");
                    }
                    strcat(query, column_query);
                }
                break;
        }
    }
    strcat(query, ");\n");
    return 0;
}
