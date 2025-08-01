#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    uint16_t size;
    char* string;
} Strpm;

#define Strpm_auto_free Strpm __attribute__((cleanup(Strpm_free)))

#define Strpm_init(name, string) Strpm name = {0};\
                                        Strpm_init_after(&name, string)\

#define Strpm_auto_free_init(name, string) Strpm name __attribute__((cleanup(Strpm_free))) = {0}; \
                                            Strpm_init_after(&name, string)\

void Strpm_free(Strpm* string) {
    if (string->string == NULL)
        return;

    free(string->string);
    string->size = 0;
    return;
}

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
