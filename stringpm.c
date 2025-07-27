#include "stringpm.h"

void stringpm_t_clear(stringpm_t* string) {
    printf("freeing value: %s", string->string);
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
    char size = 0;
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
    }

    return 0;
}
