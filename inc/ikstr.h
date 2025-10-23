//
// Created by ikryxxdev on 10/22/25.
//

#ifndef IKSTR_IKSTR_H
#define IKSTR_IKSTR_H
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h> // ssize_t TODO: Is unistd on Windows?

#define IKSTR_PREALLOC (512 * 512)

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
#ifdef __GNUC__
#define defhdr(bytes) struct __attribute__ ((__packed__)) ikstrhdr##bytes   \
{                                                                           \
    uint##bytes##_t len;                                                    \
    uint##bytes##_t cap;                                                    \
    unsigned char flags;                                                    \
    char buf[];                                                             \
}
#else
#pragma pack(push, 1)
#define defhdr(bytes) struct ikstrhdr##bytes    \
{                                               \
    uint##bytes##_t len;                        \
    uint##bytes##_t cap;                        \
    unsigned char flags;                        \
    char buf[];                                 \
};
#pragma pack(pop)
#endif

#ifdef _WIN32
#   ifdef IKSTR_EXPORTS
#       define IKSTR_API __declspec(dllexport)
#   else
#       define IKSTR_API __declspec(dllimport)
#   endif
#else
#   define IKSTR_API
#endif

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

static inline void ikstr_set_cap(ikstr s, size_t new_cap) {
    unsigned char flags = s[-1]; // get Flags
    switch (flags & IKSTR_TYPE_MASK) {
        case IKSTR_8:
            IKSTR_HDR(8,s)->cap = new_cap;
            break;
        case IKSTR_16:
            IKSTR_HDR(16,s)->cap = new_cap;
            break;
        case IKSTR_32:
            IKSTR_HDR(32,s)->cap = new_cap;
            break;
        case IKSTR_64:
            IKSTR_HDR(64,s)->cap = new_cap;
            break;
        default:
            return;
    }
}

IKSTR_API ikstr ikstr_new_len(const void* init, size_t init_len);

IKSTR_API ikstr ikstr_empty(void);

IKSTR_API ikstr ikstr_new(const char *init);

IKSTR_API ikstr ikstr_dup(ikstr s);

IKSTR_API void ikstr_free(ikstr s);

IKSTR_API ikstr ikstr_grow(ikstr s, size_t len);

IKSTR_API ikstr ikstr_concat_len(ikstr s, const void* t, size_t len);

IKSTR_API ikstr ikstr_concat(ikstr s, const char* t);

IKSTR_API ikstr ikstr_concat_ikstr(ikstr s, ikstr t);

IKSTR_API ikstr ikstr_copy_len(ikstr s, const char* t, size_t len);

IKSTR_API ikstr ikstr_copy(ikstr s, const char* t);

IKSTR_API ikstr ikstr_concat_vfmt(ikstr s, const char* fmt, va_list ap);

#ifdef __GNUC__
IKSTR_API ikstr ikstr_concat_fmt(ikstr s, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
IKSTR_API ikstr ikstr_new_fmt(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
#else
IKSTR_API ikstr ikstr_concat_fmt(ikstr s, const char* fmt, ...);
IKSTR_API ikstr ikstr_new_fmt(const char* fmt, ...);
#endif

IKSTR_API ikstr ikstr_trim(ikstr s, const char* char_set);

IKSTR_API void ikstr_range(ikstr s, ssize_t start, ssize_t end);

IKSTR_API int ikstr_cmp(ikstr s1, ikstr s2);

IKSTR_API ikstr *ikstr_split_len(const char *s, ssize_t len, const char *sep, int sep_len, int *count);

IKSTR_API ikstr *ikstr_split(const char *s, const char *sep, int *count);

IKSTR_API void ikstr_free_split_res(ikstr *tokens, int count);

IKSTR_API ikstr ikstr_join(int argc, char** argv, const char* sep);

IKSTR_API ikstr ikstr_join_ikstr(int argc, ikstr *argv, const char* sep);


// Low level funcs
IKSTR_API ikstr ikstr_make_room_for(ikstr s, size_t addlen);

#endif //IKSTR_IKSTR_H