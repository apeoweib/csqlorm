#include <stdio.h>
#include <string.h>
#include "table.h"

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


// use the Table struct to create a table in the sqlite database
void Table_create(const struct Table *table, char query[]) {
    const char create_table[] = " create table %s (";
    const char create_column[] = "%s %s";
    const char create_foreign_key[] = "foreign key(%s) references %s(%s)";
    const char is_unique[] = " unique";
    const char is_primary_key[] = " primary key";
    const char is_not_null[] = " not null";
    sprintf(query, create_table, table->name);
    for(size_t i = 0; i < table->num_columns; i++) {
        char column_def[512];
        struct Column *c = &table->columns[i];
        sprintf(column_def, create_column, c->name, type_to_str(c->type));
        if(!c->is_primary_key && c->is_unique) {
            strcat(column_def, is_unique);
        }
        if(c->is_primary_key) {
            strcat(column_def, is_primary_key);
        }
        if(!c->is_primary_key && c->is_not_null) {
            strcat(column_def, is_not_null);
        }
        if(i < table->num_columns - 1 || table->num_foreign_keys > 0) {
            strcat(column_def, ", ");
        }
        strcat(query, column_def);
    }
    for(size_t i = 0; i < table->num_foreign_keys; i++) {
        char foreign_key_def[512];
        struct ForeignKey *f = &table->foreign_keys[i];
        sprintf(foreign_key_def, create_foreign_key,
                f->src_column->name,
                f->dst_table->name,
                f->dst_column->name);
        strcat(query, foreign_key_def);
    }
    strcat(query, ");\n");
}

void Table_drop(const struct Table *table, char query[]) {
    const char drop_table[] = " drop table if exists %s;\n";
    sprintf(query, drop_table, table->name);
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

void Table_insert(char *data, struct TableMap *map, char query[]) {
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
}

