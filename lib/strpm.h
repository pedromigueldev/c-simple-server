#ifndef STRPM_H
#define STRPM_H

#include <stdint.h>

typedef struct {
    int16_t size;
    char* string;
} Strpm;

#define Strpm_auto_free Strpm __attribute__((cleanup(Strpm_free)))

#define Strpm_init(name, string) Strpm name = {0};\
                                        Strpm_init_after(&name, string)\

#define Strpm_auto_free_init(name, string) Strpm name __attribute__((cleanup(Strpm_free))) = {0}; \
                                          Strpm_init_after(&name, string)\

void Strpm_free(Strpm* string);
int Strpm_sizeof (const char* from);
int Strpm_concat (Strpm* to, Strpm* from);
int Strpm_init_after (Strpm* string, const char* value);
int Strpm_compare (Strpm* first, Strpm* second);
char* Strpm_spit (Strpm * string);
#endif
