#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/table.h"

static int count_column_maps(struct FieldMap field_maps[],
        size_t num_field_maps) {
    size_t count = 0;
    for(size_t i = 0; i < num_field_maps; i++) {
        if(field_maps[i].type == FMT_COLUMN) {
            count++;
        }
    }
    return count;
}

int asprintf_table_insert(char **query, const char * data, const struct TableMap *map) {
    const char table_insert[] = " insert into %s (%s) values (";
    const size_t table_insert_len = sizeof(table_insert);
    const char table_insert_end[] = ");\n";
    const size_t table_insert_end_len = sizeof(table_insert_end);
    const char column_sep[] = ", ";
    const size_t column_sep_len = strlen(column_sep);

    size_t column_names_len = 0;
    const size_t num_column_maps = count_column_maps(map->field_maps, map->num_field_maps);;
    size_t column_maps_added_so_far = 0;
    for(size_t i = 0; i < map->num_field_maps; i++) {
        struct FieldMap *field = &map->field_maps[i];
        switch(field->type) {
            case FMT_ARRAY:
                break;
            case FMT_COLUMN:
                {
                    column_names_len += strlen(field->column.column->name);
                    column_maps_added_so_far++;
                }
                break;
        }
        if(column_maps_added_so_far < num_column_maps)
            column_names_len += column_sep_len;
    }
    size_t values_len = 0;
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
                                values_len += strlen(column_query);
                            }
                            break;
                        case S_INT:
                            {
                                int val = *(int *)(data + field->column.offset);
                                sprintf(column_query, "%d", val);
                                values_len += strlen(column_query);
                            }
                            break;
                        case S_DOUBLE:
                            {
                                float val = *(float *)(data + field->column.offset);
                                sprintf(column_query, "%0.3f", val);
                                values_len += strlen(column_query);
                            }
                            break;
                    }
                    if(column_maps_added_so_far < num_column_maps) {
                        values_len += column_sep_len;
                    }
                }
                break;
        }
    }

    size_t size = table_insert_len + strlen(map->table->name) + column_names_len +
        values_len + table_insert_end_len;

    *query = calloc(size+1, sizeof(char));
    if(!(*query)) {
        return -1;
    }
    char column_names[512] = {0};
    column_maps_added_so_far = 0;
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
    sprintf(*query, table_insert, map->table->name, column_names);
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
                    strcat(*query, column_query);
                }
                break;
        }
    }
    strcat(*query, table_insert_end);
    return 0;
}


