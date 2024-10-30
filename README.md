# csqlorm

A basic c orm for generating sql statements for creating and destroying tables
and inserting and deleting rows.

to use the orm:
1. Define your struct
2. Define a Table object for storing the fields of the struct.  You may also
   need to define additional tables for any aggregate fields in the struct.
3. Define a TableMap object that maps struct fields to columns in the Table
   object.

## Defining Tables

define a person table:
```
struct Table person;
person.name = "person";
person.num_columns = 4;
person.columns = (struct Column[]) {
    {"id",     S_INT,    0, 1, 0},
    {"name",   S_TEXT,   1, 0, 1},
    {"age",    S_INT,    0, 0, 1},
    {"weight", S_DOUBLE, 0, 0, 1}};
person.num_foreign_keys = 0; 
```
define an item table:
```
struct Table item;
item.name = "item";
item.num_columns = 4;
item.columns = (struct Column[]) {
    {"id", S_INT, 0, 1, 0},
    {"name", S_TEXT, 1, 0, 1},
    {"count", S_INT, 0, 0, 1},
    {"person_id", S_INT, 0, 0, 0}};
item.num_foreign_keys = 1;
item.foreign_keys = (struct ForeignKey[]){
    {&item.columns[3], &person, &person.columns[0]}};
```
prepare strings for dropping and creating the tables:
```
size_t bufsize = 4096;
char person_drop_str[bufsize];
snprintf_table_drop(&person, bufsize, str);
char item_drop_str[4096];
snprintf_table_drop(&item, bufsize, str);
char person_create_str[4096];
snprintf_table_create(&person, bufsize, str);
char item_create_str[4096];
snprintf_table_create(&item, bufsize, str);
```

execute the prepared sql against the preferred database (sqlite in this
example) to generate the tables:
```
sqlite3 *db;
sqlite3_open("example.db", &db);
sqlite3_exec(db, person_drop_str, callback, 0, &error_msg);
sqlite3_exec(db, person_create_str, callback, 0, &error_msg);
sqlite3_exec(db, item_drop_str, callback, 0, &error_msg);
sqlite3_exec(db, item_create_str, callback, 0, &error_msg);
sqlite3_close(db);
```

# Defining TableMaps
```
TBD
```


# Inserting and Deleting Objects
```
TBD
```
