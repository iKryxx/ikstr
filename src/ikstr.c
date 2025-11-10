//
// Created by ikryxxdev on 10/22/25.
//

#include "ikstr.h"
#include "ikstr_alloc.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* IKSTR_NO_INIT = "__IKSTR_NO_INIT__";

static inline int ikstr_hdr_size(unsigned char type) {
    switch (type & IKSTR_TYPE_MASK) {
        case IKSTR_8:
            return sizeof(struct ikstrhdr8);
        case IKSTR_16:
            return sizeof(struct ikstrhdr16);
        case IKSTR_32:
            return sizeof(struct ikstrhdr32);
        case IKSTR_64:
            return sizeof(struct ikstrhdr64);
        default:
            return 0 ;
    }
}

static inline char ikstr_req_type(size_t len) {
    if (len < 256) return IKSTR_8;
    if (len < 65536) return IKSTR_16;
#if(LONG_MAX == LLONG_MAX)
    if (len < 4294967296) return IKSTR_32;
    return IKSTR_64;
#else
    return IKSTR_32;
#endif
}

ikstr ikstr_new_len(const void *init, size_t init_len) {
    void *sh;
    ikstr s;

    unsigned char type = ikstr_req_type(init_len);
    int hdr_len = ikstr_hdr_size(type);
    unsigned char *flag_ptr; // Pointer to the flag;

	size_t byte_size = hdr_len + init_len + 1;
	sh = iks_malloc(byte_size);
    if (NULL == sh) return NULL ;
    if (init == IKSTR_NO_INIT) init = 0;
    else if (!init) memset(sh, 0, hdr_len + init_len + 1);

    s = (char*)sh + hdr_len;
    flag_ptr = ((unsigned char*)s) - 1;
    switch (type) {
        case IKSTR_8: {
            IKSTR_HDR_VAR(8, s);
            shdr->len = init_len;
            shdr->cap = init_len;
            break;
        }
        case IKSTR_16: {
            IKSTR_HDR_VAR(16, s);
            shdr->len = init_len;
            shdr->cap = init_len;
            break;
        }
        case IKSTR_32: {
            IKSTR_HDR_VAR(32, s);
            shdr->len = init_len;
            shdr->cap = init_len;
            break;
        }
        case IKSTR_64: {
            IKSTR_HDR_VAR(64, s);
            shdr->len = init_len;
            shdr->cap = init_len;
            break;
        }
        default:
            break;
    }
    *flag_ptr = type;
    if (init_len && init) memcpy(s, init, init_len);
    s[init_len] = '\0';
    // ReSharper disable once CppDFAMemoryLeak
    return s;
}

ikstr ikstr_empty(void) {
    return ikstr_new_len("", 0);
}

ikstr ikstr_new(const char *init) {
    size_t len = NULL == init ? 0 : strlen(init);
    return ikstr_new_len(init, len);
}

ikstr ikstr_dup(ikstr s) {
    return ikstr_new_len(s, ikstr_len(s));
}

void ikstr_free(ikstr s) {
    if (NULL == s) return;
    iks_free((char*)s - ikstr_hdr_size(s[-1]));
}

ikstr ikstr_grow(ikstr s, size_t len) {
    size_t l = ikstr_len(s);

    if (len < l) return s;
    s = ikstr_make_room_for(s, len - l);
    if (NULL == s) return NULL ;
    memset(s + l, 0, len - l + 1);
    ikstr_set_len(s, len);
    return s;
}

ikstr ikstr_concat_len(ikstr s, const void *t, size_t len) {
    size_t l = ikstr_len(s);
    s = ikstr_make_room_for(s, len);
    if (NULL == s) return NULL ;
    memcpy(s + l, t, len);
    ikstr_set_len(s, l + len);
    s[l + len] = '\0';
    return s;
}

ikstr ikstr_concat(ikstr s, const char *t) {
    return ikstr_concat_len(s, t, strlen(t));
}

ikstr ikstr_concat_ikstr(ikstr s, ikstr t) {
    return ikstr_concat_len(s, t, ikstr_len(t));
}

ikstr ikstr_copy_len(ikstr s, const char *t, size_t len) {
    if (ikstr_cap(s) < len) {
        s = ikstr_make_room_for(s, len - ikstr_len(s));
        if (NULL == s) return NULL ;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    ikstr_set_len(s, len);
    return s;
}

ikstr ikstr_copy(ikstr s, const char *t) {
    return ikstr_copy_len(s, t, strlen(t));
}

ikstr ikstr_concat_vfmt(ikstr s, const char *fmt, va_list ap) {
    va_list ap2;
    char static_buf[1024], *buf = static_buf;
    size_t buf_len = strlen(fmt);
    int bufstrlen;

    // Try to use static buffer for speed
    if (buf_len > sizeof(static_buf)) {
        buf = (char*)iks_malloc(buf_len);
        if (NULL == buf) return NULL ;
    } else {
        buf_len = sizeof(static_buf);
    }

    while (1) {
        va_copy(ap2, ap);
        bufstrlen = vsnprintf(buf, buf_len, fmt, ap2);
        va_end(ap2);

        if (bufstrlen < 0) {
            if (buf != static_buf)
                iks_free(buf);
            return NULL;
        }
        if ((size_t)bufstrlen >= buf_len) {
            if (buf != static_buf)
                iks_free(buf);
            buf_len = (size_t)bufstrlen + 1;
            buf = iks_malloc(buf_len);
            if (NULL == buf) return NULL ;
            continue;
        }
        break;
    }

    char *t = ikstr_concat_len(s, buf, bufstrlen);
    if (buf != static_buf)
        iks_free(buf);
    return t;
}

ikstr ikstr_concat_fmt(ikstr s, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    s = ikstr_concat_vfmt(s, fmt, ap);
    va_end(ap);
    return s;
}

ikstr ikstr_new_fmt(const char *fmt, ...) {
    va_list ap;
    ikstr s;
    va_start(ap, fmt);
    s = ikstr_concat_vfmt(ikstr_empty(), fmt, ap);
    va_end(ap);
    return s;
}

ikstr ikstr_trim(ikstr s, const char *char_set) {
    char* str_end, *start_ptr, *end_ptr;
    size_t len;

    start_ptr = s;
    end_ptr = str_end = s + ikstr_len(s) - 1;

    while (start_ptr <= str_end && strchr(char_set, *start_ptr)) start_ptr++;
    while (end_ptr > start_ptr && strchr(char_set, *end_ptr)) end_ptr--;

    len = end_ptr - start_ptr + 1;
    if (s != start_ptr) memmove(s, start_ptr, len);
    s[len] = '\0';
    ikstr_set_len(s, len);
    return s;
}

void ikstr_range(ikstr s, ssize_t start, ssize_t end) {
    size_t new_len, len = ikstr_len(s);

    if (len == 0) return;
    if (start < 0) {
        start = (ssize_t)len + start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = (ssize_t)len + end;
        if (end < 0) end = 0;
    }
    new_len = start > end ? 0 : end - start + 1;

    if (new_len != 0) {
        if (start >= (ssize_t)len)
            new_len = 0;
        else if (end >= (ssize_t)len) {
            end = (ssize_t)len - 1;
            new_len = end - start + 1;
        }
    }

    if (start && new_len) memmove(s, s + start, new_len);
    s[new_len] = '\0';
    ikstr_set_len(s, new_len);
}

int ikstr_cmp(ikstr s1, ikstr s2) {
    size_t l1 = ikstr_len(s1), l2 = ikstr_len(s2);
    size_t min_len = l1 < l2 ? l1 : l2;
    int cmp = memcmp(s1, s2, min_len);
    if (cmp) return cmp;
    return l1 < l2 ? -1 : l1 > l2 ? 1 : 0;
}

ikstr* ikstr_split_len(const char *s, ssize_t len, const char *sep, int sep_len, int *count) {
    int elements = 0, slots = 5;
    long start = 0, j;
    ikstr *result = NULL;

    if (sep_len <= 0 || len <= 0) {
        *count = 0;
        return NULL;
    }

    result = (ikstr*)iks_malloc(sizeof(ikstr) * slots);
    if (NULL == result) return NULL;

    for (j = 0; j < len - sep_len + 1; ++j) {
        // Is there room for the next element and the final element?
        if ( slots < elements + 2) {
            ikstr *tmp;

            slots *= 2;
            tmp = (ikstr*)iks_realloc(result, sizeof(ikstr) * slots);
            if (NULL == tmp)
                goto cleanup;
            result = tmp;
        }
        // Where is the seperator?
        if ((sep_len == 1 && *(s+j) == sep[0]) || memcmp(s+j, sep, sep_len) == 0) {
            result[elements] = ikstr_new_len(s + start, j - start);
            if (result[elements] == NULL) goto cleanup;
            ++elements;
            start = j + sep_len;
            j += sep_len - 1; // Skip the seperator
        }

    }
    result[elements] = ikstr_new_len(s + start, len - start);
    if (result[elements] == NULL) goto cleanup;
    ++elements;
    *count = elements;
    return result;

    cleanup:
    {
        for (int i = 0; i < elements; ++i) ikstr_free(result[i]);
        iks_free(result);
        *count = 0;
        return NULL;
    }
}

ikstr* ikstr_split(const char *s, const char *sep, int *count) {
    return ikstr_split_len(s, strlen(s), sep, strlen(sep), count);
}

void ikstr_free_split_res(ikstr *tokens, int count) {
    if (!tokens) return;
    while (count--) ikstr_free(tokens[count]);
    iks_free(tokens);
}

ikstr ikstr_join(int argc, char **argv, const char *sep) {
    ikstr s = ikstr_empty();

    for (int i = 0; i < argc; ++i) {
        s = ikstr_concat(s, argv[i]);
        if (i < argc - 1) s = ikstr_concat(s, sep);
    }
    return s;
}

ikstr ikstr_join_ikstr(int argc, ikstr* argv, const char *sep) {
    ikstr s = ikstr_empty();

    for (int i = 0; i < argc; ++i) {
        s = ikstr_concat_ikstr(s, argv[i]);
        if (i < argc - 1) s = ikstr_concat(s, sep);
    }
    return s;
}

ikstr ikstr_make_room_for(ikstr s, size_t addlen) {
    void *sh, *new_sh;
    size_t avail = ikstr_avail(s);
    size_t len, new_len, req_len;
    char type, old_type = (char)(s[-1] & IKSTR_TYPE_MASK);
    int hdr_len;

    // No need to do anything
    if (avail >= addlen) return s;

    len = ikstr_len(s);
    sh = (char*)s - ikstr_hdr_size(old_type);
    req_len = new_len = (len + addlen);

    if (new_len < IKSTR_PREALLOC)
        new_len *= 2;
    else
        new_len += IKSTR_PREALLOC;

    type = ikstr_req_type(new_len);
    hdr_len = ikstr_hdr_size(type);

    if (hdr_len + new_len + 1 <= req_len) // size_t overflow
        return NULL ;

    if (old_type == type) {
        new_sh = iks_realloc(sh, hdr_len + new_len + 1);
        // ReSharper disable once CppDFAMemoryLeak
        if (NULL == new_sh) return NULL ;
        s = (char*)new_sh + hdr_len;
    }
    else {
        new_sh = iks_malloc(hdr_len + new_len + 1);
        // ReSharper disable once CppDFAMemoryLeak
        if (NULL == s) return NULL ;
        memcpy((char*) new_sh + hdr_len, s, len + 1);
        iks_free(sh);

        s = (char*)new_sh + hdr_len;
        s[-1] = type;
        ikstr_set_len(s, len);
    }
    ikstr_set_cap(s, new_len);
    // ReSharper disable once CppDFAMemoryLeak
    return s;
}