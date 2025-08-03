#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "./strpm.h"

void Strpm_free(Strpm* string) {
    if (string->string == NULL)
        return;

    free(string->string);
    string->size = 0;
    string->string = NULL;
    return;
}

char* Strpm_spit(Strpm * string) {
    char* out = malloc((string->size + 1) * sizeof(char));
    for (int i = 0; i < string->size; i++) {
        out[i] = string->string[i];
    }
    out[string->size] = '\0';
    return out;
};

int Strpm_sizeof (const char* from) {
    size_t size = 0;
    while (*from) {
        size++;from++;
    }
    return size;
};


int Strpm_concat (Strpm* to, Strpm* from) {
    if (to->size <= 0) {
        to->string = malloc(sizeof(char*)*from->size);
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

int Strpm_init_after (Strpm* string, const char* value) {
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
    }
    // else {
    //     if (string->size < size){
    //         string->size += size;
    //         string->string = realloc(string->string, sizeof(char)*string->size);
    //     }

    //     const char* c = (value-size);
    //     for (int i = 0; i < string->size; i++) {
    //         string->string[i] = c[i];
    //     }
    // }

    return 0;
}

int Strpm_compare (Strpm* first, Strpm* second) {
    if (first->size != second->size) {
        return 1;
    }
    for (int i = 0; i < first->size; i++) {
        if (first->string[i] != second->string[i]) {
            return 1;
        }
    }
    return 0;
}
