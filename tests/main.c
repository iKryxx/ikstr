//
// Created by ikryxxdev on 10/22/25.
//

#include <stdio.h>
#include <ikstr.h>
#include <stdlib.h>

int main(void) {
    ikstr s = ikstr_new("+##+ .. ..+#+#+ Hello, World! .-.--#+#+#----...");
    s = ikstr_trim(s, ".-#+ ");
    printf("%s\n", s);
    ikstr_range(s, 1, -1);
    printf("%s\n", s);
    ikstr_free(s);

    s = ikstr_new("Hello, I am a great Developer! I love C.");
    int count;
    ikstr *toks = ikstr_split(s, " ", &count);
    for (int i = 0; i < count; ++i) {
        toks[i] = ikstr_trim(toks[i], ".,!");
        printf("%d. %s\n", i+1, toks[i]);
    }
    ikstr_free_split_res(toks, count);
    ikstr_free(s);

    char* list[] = {"Apples", "Bananas", "Strawberries"};
    s = ikstr_join(3, list, ", ");
    printf("My favorite fruits are %s.", s);
    ikstr_free(s);

    return 0;
}
