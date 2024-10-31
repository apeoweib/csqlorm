#include <stdlib.h>
#include <string.h>
#include "../include/table.h"


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


