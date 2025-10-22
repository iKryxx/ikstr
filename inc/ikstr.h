//
// Created by ikryxxdev on 10/22/25.
//

#ifndef IKSTR_IKSTR_H
#define IKSTR_IKSTR_H
#include <stddef.h>
#include <stdint.h>

typedef char *ikstr;

extern char* IKSTR_NO_INIT;


/*
 * Defines the structures for the headers given the bytes
 *
 * The header struct is packed, and has these fields:
 * <blui> len
 * <blui> cap
 * unsigned char flags
 * char* buf
 *
 * where <blui> stands for an unsigned integer with size <bytes>
 */
#define defhdr(bytes) struct __attribute__ ((__packed__)) ikstrhdr##bytes {uint##bytes##_t len; uint##bytes##_t cap; unsigned char flags; char buf[];}

defhdr(8);
defhdr(16);
defhdr(32);
defhdr(64);

#undef defhdr

enum {
    IKSTR_8         = 0,
    IKSTR_16        = 1,
    IKSTR_32        = 2,
    IKSTR_64        = 3,
    IKSTR_TYPE_MASK = 7,
    IKSTR_TYPE_BITS = 3,
};

#define IKSTR_HDR_VAR(T, s) struct ikstrhdr##T *shdr = (void*)((s) - sizeof(struct ikstrhdr##T));
#define IKSTR_HDR(T, s) ((struct ikstrhdr##T *)((s) - (sizeof(struct ikstrhdr##T))))

static inline size_t ikstr_len(ikstr s) {
    const unsigned char flags = s[-1]; // get Flags
    switch (flags & IKSTR_TYPE_MASK) {
        case IKSTR_8:
            return IKSTR_HDR(8, s)->len;
        case IKSTR_16:
            return IKSTR_HDR(16, s)->len;
        case IKSTR_32:
            return IKSTR_HDR(32, s)->len;
        case IKSTR_64:
            return IKSTR_HDR(64, s)->len;
        default:
            return 0;
    }
    return 0;
}

static inline size_t ikstr_avail(ikstr s) {
    const unsigned char flags = s[-1]; // get Flags
    switch (flags & IKSTR_TYPE_MASK) {
        case IKSTR_8: {
            IKSTR_HDR_VAR(8, s);
            return shdr->cap - shdr->len;
        }
        case IKSTR_16: {
            IKSTR_HDR_VAR(16, s);
            return shdr->cap - shdr->len;
        }
        case IKSTR_32: {
            IKSTR_HDR_VAR(32, s);
            return shdr->cap - shdr->len;
        }
        case IKSTR_64: {
            IKSTR_HDR_VAR(64, s);
            return shdr->cap - shdr->len;
        }
        default:
            return 0;
    }
    return 0;
}

static inline void ikstr_set_len(ikstr s, size_t new_len) {
    unsigned char flags = s[-1]; // get Flags
    switch (flags & IKSTR_TYPE_MASK) {
        case IKSTR_8:
            IKSTR_HDR(8,s)->len = new_len;
            break;
        case IKSTR_16:
            IKSTR_HDR(16,s)->len = new_len;
            break;
        case IKSTR_32:
            IKSTR_HDR(32,s)->len = new_len;
            break;
        case IKSTR_64:
            IKSTR_HDR(64,s)->len = new_len;
            break;
        default:
            return;
    }
}

static inline void ikstr_inc_len(ikstr s, size_t inc) {
    unsigned char flags = s[-1]; // get Flags
    switch (flags & IKSTR_TYPE_MASK) {
        case IKSTR_8:
            IKSTR_HDR(8,s)->len += inc;
            break;
        case IKSTR_16:
            IKSTR_HDR(16,s)->len += inc;
            break;
        case IKSTR_32:
            IKSTR_HDR(32,s)->len += inc;
            break;
        case IKSTR_64:
            IKSTR_HDR(64,s)->len += inc;
            break;
        default:
            return;
    }
}

static inline size_t ikstr_cap(ikstr s) {
    const unsigned char flags = s[-1]; // get Flags
    switch (flags & IKSTR_TYPE_MASK) {
        case IKSTR_8:
            return IKSTR_HDR(8, s)->cap;
        case IKSTR_16:
            return IKSTR_HDR(16, s)->cap;
        case IKSTR_32:
            return IKSTR_HDR(32, s)->cap;
        case IKSTR_64:
            return IKSTR_HDR(64, s)->cap;
        default:
            return 0;
    }
    return 0;
}

static inline void ikstr_set_cap(ikstr s, size_t new_len) {
    unsigned char flags = s[-1]; // get Flags
    switch (flags & IKSTR_TYPE_MASK) {
        case IKSTR_8:
            IKSTR_HDR(8,s)->len = new_len;
            break;
        case IKSTR_16:
            IKSTR_HDR(16,s)->len = new_len;
            break;
        case IKSTR_32:
            IKSTR_HDR(32,s)->len = new_len;
            break;
        case IKSTR_64:
            IKSTR_HDR(64,s)->len = new_len;
            break;
        default:
            return;
    }
}

ikstr ikstr_new_len(const void* init, size_t init_len);

ikstr ikstr_empty(void);

ikstr ikstr_new(const char *init);

ikstr ikstr_dup(ikstr s);

void ikstr_free(ikstr s);

#endif //IKSTR_IKSTR_H