//
// Created by ikryxxdev on 10/22/25.
//

#include "ikstr.h"
#include "ikstr_alloc.h"
#include <limits.h>
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
            return 0;
    }
}

static inline unsigned char ikstr_req_type(size_t len) {
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
    if (0 == sh) return 0;
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
    }
    *flag_ptr = type;
    if (init_len && init) memcpy(s, init, init_len);
    s[init_len] = '\0';
    return s;

}

ikstr ikstr_empty(void) {
    return ikstr_new_len("", 0);
}

ikstr ikstr_new(const char *init) {
    size_t len = 0 == init ? 0 : strlen(init);
    return ikstr_new_len(init, len);
}

ikstr ikstr_dup(ikstr s) {
    return ikstr_new_len(s, ikstr_len(s));
}

void ikstr_free(ikstr s) {
    if (0 == s) return;
    iks_free((char*)s - ikstr_hdr_size(s[-1]));
}
