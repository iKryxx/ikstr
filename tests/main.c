//
// Created by ikryxxdev on 10/22/25.
//

#include <stdio.h>
#include <ikstr.h>
#include <stdlib.h>

int main(void) {
    ikstr s = ikstr_new("Hello");
    ikstr_concat(s, ", World!");
    printf("%s\n", s);
    ikstr_free(s);
    return 0;
}
