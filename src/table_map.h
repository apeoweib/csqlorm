#ifndef TABLE_MAP_H
#define TABLE_MAP_H
#include <stdlib.h>

enum CType {
    C_FLOAT,
    C_DOUBLE,
    C_INT,
    C_STR
};

// map a field in a struct onto a single column in a table
struct ColumnMap {
    struct Column *column;
    enum CType c_type; // the type of the field
    int offset;
};

struct ArrayMap {
    struct TableMap *map;
    struct ForeignKey *foreign_key;  // the foreign key column in "table"
    int array_offset;
    int array_size_offset;
    int size;  // the size of an array element in bytes
};

enum FieldMapType {
    FMT_COLUMN
    , FMT_ARRAY
//    , FMT_TABLE
};

struct FieldMap {
    enum FieldMapType type;
    union {
        struct ColumnMap column;
        struct ArrayMap array;
    };
};

// defines a mapping of the fields of a struct onto the columns
// of tables in a database.  This mapping allows the values from
// any struct to be inserted into the database.  More than one
// mapping can be defined for the same struct type.  The mapping
// is recursive, allowing fields of a struct that are themselves
// structs, to be mapped.   It also allows arrays of values to
// be mapped.
struct TableMap {
    struct Table *table;
    struct FieldMap *field_maps;
    size_t num_field_maps;
};

#endif
