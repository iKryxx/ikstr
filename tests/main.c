//
// Created by ikryxxdev on 10/22/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <ikstr.h>
#include <string.h>

/**
 * Helper to print an ikstr with its length and capacity.
 */
static void print_ikstr(const char *label, ikstr s) {
    printf("%s: \"%s\" (len=%zu, cap=%zu, avail=%zu)\n",
           label, s ? s : "(null)", s ? ikstr_len(s) : 0, s ? ikstr_cap(s) : 0, s ? ikstr_avail(s) : 0);
}

/**
 * Demonstrates ikstr_new_len and ikstr_free.
 * - Creates strings with explicit lengths and initialization forms.
 */
void ex_new_len_and_free(void) {
    const char raw[] = "hello world";
    ikstr a = ikstr_new_len(raw, 5);                 // "hello"
    ikstr b = ikstr_new_len(NULL, 8);                 // zero-initialized 8 bytes -> empty visible string
    ikstr c = ikstr_new_len(IKSTR_NO_INIT, 4);        // uninitialized payload (still gets a terminating '\0')

    print_ikstr("a", a);
    print_ikstr("b", b);
    print_ikstr("c", c);

    ikstr_free(a);
    ikstr_free(b);
    ikstr_free(c);
}

/**
 * Demonstrates ikstr_empty and ikstr_new.
 */
void ex_new_and_empty(void) {
    ikstr s1 = ikstr_empty();              // ""
    ikstr s2 = ikstr_new("sample text");   // "sample text"
    ikstr s3 = ikstr_new(NULL);            // ""

    print_ikstr("s1", s1);
    print_ikstr("s2", s2);
    print_ikstr("s3", s3);

    ikstr_free(s1);
    ikstr_free(s2);
    ikstr_free(s3);
}

/**
 * Demonstrates ikstr_dup.
 */
void ex_dup(void) {
    ikstr src = ikstr_new("duplicate me");
    ikstr copy = ikstr_dup(src);

    print_ikstr("src", src);
    print_ikstr("copy", copy);

    ikstr_free(src);
    ikstr_free(copy);
}

/**
 * Demonstrates ikstr_grow and capacity/length inspection.
 */
void ex_grow_and_accessors(void) {
    ikstr s = ikstr_new("abc");
    print_ikstr("initial", s);

    s = ikstr_grow(s, 10); // extend logical length and zero-fill new region
    print_ikstr("after grow to len=10", s);

    // Fill the grown region and keep null terminator
    for (size_t i = 3; i < ikstr_len(s); ++i) s[i] = 'x';
    s[ikstr_len(s)] = '\0';
    print_ikstr("after fill", s);

    ikstr_free(s);
}

/**
 * Demonstrates ikstr_concat_len, ikstr_concat, ikstr_concat_ikstr.
 */
void ex_concat_variants(void) {
    ikstr base = ikstr_new("foo");
    const char tail[] = "BAR!!!";

    base = ikstr_concat_len(base, tail, 3); // append "BAR" (first 3 bytes)
    print_ikstr("after concat_len", base);

    base = ikstr_concat(base, "-baz");
    print_ikstr("after concat cstr", base);

    ikstr other = ikstr_new("_qux");
    base = ikstr_concat_ikstr(base, other);
    print_ikstr("after concat ikstr", base);

    ikstr_free(base);
    ikstr_free(other);
}

/**
 * Demonstrates ikstr_copy_len and ikstr_copy.
 */
void ex_copy_variants(void) {
    ikstr s = ikstr_new("initial");

    s = ikstr_copy_len(s, "abcdefgh", 5); // "abcde"
    print_ikstr("after copy_len", s);

    s = ikstr_copy(s, "new contents here");
    print_ikstr("after copy", s);

    ikstr_free(s);
}

/**
 * Demonstrates ikstr_concat_fmt, ikstr_concat_vfmt (indirect), and ikstr_new_fmt.
 */
void ex_formatting(void) {
    ikstr s = ikstr_new("Count: ");
    s = ikstr_concat_fmt(s, "%d + %d = %d", 2, 3, 5);
    print_ikstr("concat_fmt", s);

    ikstr t = ikstr_new_fmt("Pi approx: %.2f, hex: 0x%X", 3.14159, 48879);
    print_ikstr("new_fmt", t);

    ikstr_free(s);
    ikstr_free(t);
}

/**
 * Demonstrates ikstr_trim.
 */
void ex_trim(void) {
    ikstr s = ikstr_new(" \t\n--hello world--\n ");
    print_ikstr("before", s);

    s = ikstr_trim(s, " \t\n-"); // trim spaces, tabs, newlines, and '-'
    print_ikstr("after trim", s);

    ikstr_free(s);
}

/**
 * Demonstrates ikstr_range.
 */
void ex_range(void) {
    ikstr s = ikstr_new("0123456789");
    print_ikstr("original", s);

    ikstr_range(s, 2, 6); // keep "23456"
    print_ikstr("range(2,6)", s);

    ikstr_range(s, -3, -1); // from end: last 3 -> "456" -> then range on current -> becomes "456"
    print_ikstr("range(-3,-1)", s);

    ikstr_range(s, 5, 2); // invalid -> empty
    print_ikstr("range(5,2)", s);

    ikstr_free(s);
}

/**
 * Demonstrates ikstr_cmp.
 */
void ex_compare(void) {
    ikstr a = ikstr_new("abc");
    ikstr b = ikstr_new("abd");
    ikstr c = ikstr_new("abc");

    printf("cmp(a,b) = %d\n", ikstr_cmp(a, b)); // negative
    printf("cmp(b,a) = %d\n", ikstr_cmp(b, a)); // positive
    printf("cmp(a,c) = %d\n", ikstr_cmp(a, c)); // zero

    ikstr_free(a);
    ikstr_free(b);
    ikstr_free(c);
}

/**
 * Demonstrates ikstr_split_len, ikstr_split, and ikstr_free_split_res.
 */
void ex_split_and_free(void) {
    const char *text = "a::bb::ccc::";
    int count = 0;

    // split_len with "::"
    ikstr *tokens = ikstr_split_len(text, (ssize_t)strlen(text), "::", 2, &count);
    printf("split_len count=%d\n", count);
    for (int i = 0; i < count; ++i) {
        print_ikstr("token", tokens[i]);
    }
    ikstr_free_split_res(tokens, count);

    // split with ","
    int count2 = 0;
    ikstr *t2 = ikstr_split("x,y,,z", ",", &count2);
    printf("split count=%d\n", count2);
    for (int i = 0; i < count2; ++i) {
        print_ikstr("token", t2[i]);
    }
    ikstr_free_split_res(t2, count2);
}

/**
 * Demonstrates ikstr_join with C strings.
 */
void ex_join_cstr(void) {
    const char *arr[] = {"one", "two", "three"};
    ikstr j = ikstr_join(3, (char**)arr, " | ");
    print_ikstr("join cstr", j);
    ikstr_free(j);
}

/**
 * Demonstrates ikstr_join_ikstr with ikstr array.
 */
void ex_join_ikstr(void) {
    ikstr parts[4];
    parts[0] = ikstr_new("red");
    parts[1] = ikstr_new("green");
    parts[2] = ikstr_new("blue");
    parts[3] = ikstr_new("alpha");

    ikstr j = ikstr_join_ikstr(4, parts, "/");
    print_ikstr("join ikstr", j);

    for (int i = 0; i < 4; ++i) ikstr_free(parts[i]);
    ikstr_free(j);
}

/**
 * Demonstrates ikstr_make_room_for by reserving capacity before appends.
 */
void ex_make_room_for(void) {
    ikstr s = ikstr_new("seed");
    print_ikstr("initial", s);

    s = ikstr_make_room_for(s, 1000); // reserve
    print_ikstr("after reserve", s);

    // Append a few times without frequent reallocations
    for (int i = 0; i < 5; ++i) {
        s = ikstr_concat(s, "_chunk");
    }
    print_ikstr("after appends", s);

    ikstr_free(s);
}

/**
 * Entry point to run all examples.
 */
int main(void) {
    ex_new_len_and_free();
    ex_new_and_empty();
    ex_dup();
    ex_grow_and_accessors();
    ex_concat_variants();
    ex_copy_variants();
    ex_formatting();
    ex_trim();
    ex_range();
    ex_compare();
    ex_split_and_free();
    ex_join_cstr();
    ex_join_ikstr();
    ex_make_room_for();
    return 0;
}
