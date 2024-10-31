example:
	mkdir -p bin
	clang -o bin/example -Iinclude -Ilib/sqlite3/include -pg examples/example.c lib/sqlite/sqlite3.c src/snprintf_table_create.c src/snprintf_table_drop.c src/snprintf_table_insert.c src/asprintf_table_insert.c src/asprintf_table_create.c src/asprintf_table_drop.c
