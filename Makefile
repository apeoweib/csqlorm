example:
	mkdir -p bin
	clang -o bin/example -Iinclude -Ilib/sqlite3/include -pg examples/example.c lib/sqlite/sqlite3.c src/table.c
