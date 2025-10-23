//
// Created by ikryxxdev on 10/22/25.
//

#include <stdio.h>
#include <ikstr.h>
#include <stdlib.h>

int main(void) {
    ikstr s = ikstr_new_fmt("Sum of %d and %d is %d", 3, 3, 6);
    printf("%s\n", s);
    ikstr_free(s);
    return 0;
}
