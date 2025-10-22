//
// Created by ikryxxdev on 10/22/25.
//

#include <stdio.h>
#include <ikstr.h>

int main(void) {
    ikstr s = ikstr_new("Hello world!");
    printf("%s", s);
    ikstr_free(s);
    return 0;
}
