#include "stringpm.h"
#include <stddef.h>

void stringpm_t_free(stringpm_t* string) {
    static int count = 0;
    printf("freeing string number: %d\n", ++count);
    if (string->string == NULL)
        return;

    free(string->string);
    string->size = 0;
    return;
}

int stringpm_t_concat (stringpm_t* to, stringpm_t* from) {
    if (to->size <= 0) {
        to->string = malloc(sizeof(char)*from->size);
        to->size = from->size;
        for (int i = 0; i < from->size; i++) {
            to->string[i] = from->string[i];
        }
        return 0;
    }

    int old_size = to->size;
    to->size = old_size + from->size;
    to->string = realloc(to->string, to->size);

    for (int i = 0; i < from->size; i++)
        to->string[i + old_size] = from->string[i];

    return 0;
}

int stringpm_t_init (stringpm_t* string, const char* value) {
    size_t size = 0;
    while (*value) {
        size++;value++;
    }

    if (string->string == NULL) {
        string->string = malloc(sizeof(char)*size);
        string->size = size;

        const char* c = (value-size);
        for (int i = 0; i < string->size; i++) {
            string->string[i] = c[i];
        }
    } else {
        if (string->size < size){
            string->size += size;
            string->string = realloc(string->string, sizeof(char)*string->size);
        }

        const char* c = (value-size);
        for (int i = 0; i < string->size; i++) {
            string->string[i] = c[i];
        }
    }

    return 0;
}
