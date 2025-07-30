#ifndef STRINGPM_H
#define STRINGPM_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define stringpm_t_auto_free(name) stringpm_t name __attribute__((cleanup(stringpm_t_free)))

typedef struct {
    uint16_t size;
    char* string;
} stringpm_t;

int stringpm_t_concat (stringpm_t* to, stringpm_t* from);
int stringpm_t_init (stringpm_t* string, const char* value);
void stringpm_t_free(stringpm_t* string);

#endif
