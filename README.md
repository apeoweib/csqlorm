# csqlorm

A basic c orm for generating sql statements for creating and destroying tables
and inserting and deleting rows.

This is a work in progress.

## Defining Tables

define a person table:
```
struct Table person;
person.name = "person";
person.num_columns = 4;
struct Column c1 = {"id", S_INT, 0, 1, 0};
struct Column c2 = {"name", S_TEXT, 1, 0, 1};
struct Column c3 = {"age", S_INT, 0, 0, 1};
struct Column c4 = {"weight", S_DOUBLE, 0, 0, 1};
person.columns = (struct Column[]){c1, c2, c3, c4};
person.num_foreign_keys = 0; 
```
define an item table:
```
struct Table item;
item.name = "item";
item.num_columns = 4;
struct Column c5 = {"id", S_INT, 0, 1, 0};
struct Column c6 = {"name", S_TEXT, 1, 0, 1};
struct Column c7 = {"count", S_INT, 0, 0, 1};
struct Column c8 = {"person_id", S_INT, 0, 0, 0};
item.columns = (struct Column[]){c5, c6, c7, c8};
struct ForeignKey f = {&c8, &persons, &c1};
item.num_foreign_keys = 1;
item.foreign_keys = (struct ForeignKey[]){f};
```
prepare strings for dropping and creating the tables:
```
char person_drop_str[4096];
Table_drop(&person, str);
char item_drop_str[4096];
Table_drop(&item, str);
char person_create_str[4096];
Table_create(&person, str);
char item_create_str[4096];
Table_create(&item, str);
```

execute the prepared sql against the preferred database:
```
sqlite3 *db;
sqlite3_open("example.db", &db);
sqlite3_exec(db, person_drop_str, callback, 0, &error_msg);
sqlite3_exec(db, person_create_str, callback, 0, &error_msg);
sqlite3_exec(db, item_drop_str, callback, 0, &error_msg);
sqlite3_exec(db, item_create_str, callback, 0, &error_msg);
sqlite3_close(db);
```
