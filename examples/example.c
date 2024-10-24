#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "sqlite3.h"
#include "table.h"
#include "table_map.h"

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
    struct Item *items;
    size_t num_items;
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

    struct Table persons;
    persons.name = "person";
    persons.num_columns = 4;
    struct Column c1 = {"id", S_INT, 0, 1, 0};
    struct Column c2 = {"name", S_TEXT, 1, 0, 1};
    struct Column c3 = {"age", S_INT, 0, 0, 1};
    struct Column c4 = {"weight", S_DOUBLE, 0, 0, 1};
    persons.columns = (struct Column[]){c1, c2, c3, c4};
    persons.num_foreign_keys = 0; 

    struct Table items;
    items.name = "item";
    items.num_columns = 4;
    struct Column c5 = {"id", S_INT, 0, 1, 0};
    struct Column c6 = {"name", S_TEXT, 1, 0, 1};
    struct Column c7 = {"count", S_INT, 0, 0, 1};
    struct Column c8 = {"person_id", S_INT, 0, 0, 0};
    items.columns = (struct Column[]){c5, c6, c7, c8};
    struct ForeignKey f = {&c8, &persons, &c1};
    items.num_foreign_keys = 1;
    items.foreign_keys = (struct ForeignKey[]){f};

    char str[4096];
    Table_drop(&persons, str);
    printf("table drop:\n%s\n", str);
    exec(db, str);
    Table_drop(&items, str);
    printf("table drop:\n%s\n", str);
    exec(db, str);
    Table_create(&persons, str);
    printf("table create:\n%s\n", str);
    exec(db, str);
    Table_create(&items, str);
    printf("table create:\n%s\n", str);
    exec(db, str);

    struct Person person_struct = {"Fred", 27, 120.0, NULL, 0};
    person_struct.num_items = 1;
    person_struct.items = (struct Item[]){{"sword", 1}};

    struct Item item_struct = {"cheese", 1};

    struct TableMap item_map;
    item_map.table = &items;
    item_map.num_field_maps = 2;
    item_map.field_maps = (struct FieldMap[]) {
        {FMT_COLUMN, {{&items.columns[1], C_STR, offsetof(struct Item, name)}}},
        {FMT_COLUMN, {{&items.columns[2], C_INT, offsetof(struct Item, count)}}}
    };

    // inserting values from a struct requires that we define a mapping from 
    // the struct to the table(s)
    struct TableMap person_map;
    person_map.table = &persons;
    person_map.num_field_maps = 4;
    person_map.field_maps = (struct FieldMap[]){
        {FMT_COLUMN, {{&persons.columns[1], C_STR, offsetof(struct Person, name)}}},
        {FMT_COLUMN, {{&persons.columns[2], C_INT, offsetof(struct Person, int_value)}}},
        {FMT_COLUMN, {{&persons.columns[3], C_FLOAT, offsetof(struct Person, float_value)}}},
        {FMT_ARRAY, .array={&item_map, &item_map.table->foreign_keys[0], offsetof(struct Person, items), offsetof(struct Person, num_items)}}
    };

    // inserting an item directly into the table using the item map.  THe
    // foreign key reference to the person table wil be null.
    Table_insert((char *)&item_struct, &item_map, str);
    printf("%s table insert:\n%s\n", items.name, str);
    exec(db, str);

    // inserting a person with one item into the person table using the person map.
    // The person's item should also be entered into the item table, with the
    // correct foreign key reference to the newly created person.  If there
    // is an error in inserting the item (such as a violation of a
    // uniqueness constraint) the person should also not be entered, i.e.
    // the transaction should be atomic.
    Table_insert((char *)&person_struct, &person_map, str);
    printf("%s table insert:\n%s\n", persons.name, str);
    exec(db, str);

    sqlite3_close(db);
    return 0;
}
