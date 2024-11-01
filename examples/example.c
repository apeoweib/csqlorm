#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "sqlite3.h"
#include "../include/table.h"
#include "../include/table_map.h"

// we need a data type that can describe a mapping of a struct
// that contains another struct, onto two separate tables, as well
// as fields of a struct onto columns in a table.
//
// We can define, for a given struct, and a collection of tables,
// a map that specifies how one or more fields of the struct map
// to the fields in one or more tables, but can also include submaps
// that map struct fields (which could be a struct, or an array of
// structs) to a separate table.
//
// struct StructToTablesMap
// [{int/double/text, table *, column *}]
// [{struct, offset, StructToTablesMap}]
// [{struct_array, struct_offset, count_offset, StructToTablesMap}]

struct Item {
    char *name;
    int count;
};

// an example type for which instances will saved to, and read from, a table
struct Person {
    char *name;
    int int_value;
    float float_value;
    struct Item *item;
    size_t num_item;
};

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    (void)NotUsed;
    int i;
    for(i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void exec(sqlite3 *db, const char *str) {
    char *error_msg = 0;
    int result = sqlite3_exec(db, str, callback, 0, &error_msg);
    if(result != SQLITE_OK) {
        fprintf(stderr, "%s\n", error_msg);
        sqlite3_free(error_msg);
        exit(-1);
    }
}


int main(void) {

    sqlite3 *db;
    sqlite3_open("example.db", &db);

    struct Table person;
    person.name = "person";
    person.num_columns = 4;
    person.columns = (struct Column[]) {
        {"id",     S_INT,    CC_PRIMARY_KEY},
        {"name",   S_TEXT,   CC_UNIQUE | CC_NOT_NULL},
        {"age",    S_INT,    CC_NOT_NULL},
        {"weight", S_DOUBLE, CC_NOT_NULL}};
    person.num_foreign_keys = 0; 

    struct Table item;
    item.name = "item";
    item.num_columns = 4;
    item.columns = (struct Column[]) {
        {"id", S_INT, CC_PRIMARY_KEY},
            {"name", S_TEXT, CC_UNIQUE | CC_NOT_NULL},
            {"count", S_INT, CC_NOT_NULL},
            {"person_id", S_INT, CC_NONE}}; 
    item.num_foreign_keys = 1;
    item.foreign_keys = (struct ForeignKey[]){
        {&item.columns[3], &person, &person.columns[0]}};

    size_t buflen = 4096;
    char str[buflen];
    snprintf_table_drop(str, 4096, &person);
    printf("table drop:\n%s\n", str);
    exec(db, str);
    snprintf_table_drop(str, 4096, &item);
    printf("table drop:\n%s\n", str);
    exec(db, str);
    snprintf_table_create(str, 4096, &person);
    printf("table create:\n%s\n", str);
    exec(db, str);
    snprintf_table_create(str, 4096, &item);
    printf("table create:\n%s\n", str);
    exec(db, str);

    struct Person person_struct = {"Fred", 27, 120.0, NULL, 0};
    person_struct.num_item = 1;
    person_struct.item = (struct Item[]){{"sword", 1}};

    struct Item item_struct = {"cheese", 1};

    struct TableMap item_map;
    item_map.table = &item;
    item_map.num_field_maps = 2;
    item_map.field_maps = (struct FieldMap[]) {
        {FMT_COLUMN, {{&item.columns[1], C_STR, offsetof(struct Item, name)}}},
        {FMT_COLUMN, {{&item.columns[2], C_INT, offsetof(struct Item, count)}}}
    };

    // inserting values from a struct requires that we define a mapping from 
    // the struct to the table(s)
    struct TableMap person_map;
    person_map.table = &person;
    person_map.num_field_maps = 4;
    person_map.field_maps = (struct FieldMap[]){
        {FMT_COLUMN, {{&person.columns[1], C_STR, offsetof(struct Person, name)}}},
        {FMT_COLUMN, {{&person.columns[2], C_INT, offsetof(struct Person, int_value)}}},
        {FMT_COLUMN, {{&person.columns[3], C_FLOAT, offsetof(struct Person, float_value)}}},
        {FMT_ARRAY, .array={&item_map, &item_map.table->foreign_keys[0], offsetof(struct Person, item), offsetof(struct Person, num_item)}}
    };

    snprintf_table_insert(str, 4096, (char *)&item_struct, &item_map);
    printf("%s table insert:\n%s\n", item.name, str);
    exec(db, str);

    snprintf_table_insert(str, 4096, (char *)&person_struct, &person_map);
    printf("%s table insert:\n%s\n", person.name, str);
    exec(db, str);

    sqlite3_close(db);
    return 0;
}
