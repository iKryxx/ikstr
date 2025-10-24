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

/**
 * Represents a special initialization flag used to indicate no initialization for an `ikstr`.
 *
 * This string constant is used internally within the ikstr library to signify cases where
 * an ikstr instance should be allocated without performing any initialization of its data.
 * It typically identifies the absence of a default or provided initial value while
 * creating an ikstr.
 *
 * When passed as an argument where initialization data is expected, `IKSTR_NO_INIT` prevents
 * any default initialization and ensures the raw memory is allocated without being modified.
 */
extern char* IKSTR_NO_INIT;


#ifdef __GNUC__
/**
 * Macro: defhdr(bytes)
 *
 * Defines a packed header structure for ikstr with integer fields sized by the
 * provided byte width. The resulting struct is named `ikstrhdr<bytes>`.
 *
 * Layout (packed):
 *   - uint<bytes>_t len   : Current string length (number of bytes before the '\0').
 *   - uint<bytes>_t cap   : Usable capacity (excluding the trailing '\0').
 *   - unsigned char flags : Internal flags; low IKSTR_TYPE_BITS bits encode the header width.
 *   - char buf[]          : Flexible array member; start of the character buffer.
 *
 * Notes:
 * - The structure is declared packed to ensure predictable layout across compilers.
 *   - GCC/Clang: uses __attribute__((__packed__)).
 *   - Other compilers: uses pragma pack push/pop with alignment 1.
 * - The `bytes` argument must be one of: 8, 16, 32, 64. It selects the width
 *   of the length and capacity fields via the fixed-width integer types:
 *   uint8_t, uint16_t, uint32_t, uint64_t.
 * - The flexible array member `buf[]` directly follows the header in memory and
 *   is the storage backing for the public `ikstr` pointer (which points to `buf`).
 * - Instances of these headers are not created directly by users. They are
 *   allocated internally by the ikstr creation/growth routines. User code
 *   interacts with `ikstr` as a `char*` and obtains metadata via helper macros
 *   and inline accessors (e.g., ikstr_len, ikstr_cap).
 *
 * Example (conceptual use inside library code):
 *   defhdr(8);  // declares struct ikstrhdr8 { uint8_t len; uint8_t cap; ...; char buf[]; };
 *
 * Safety:
 * - Because the headers are packed, direct field access is done carefully by
 *   the library to avoid alignment issues. User code should not rely on the
 *   internal layout beyond the documented fields.
 */
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


/**
 * Returns the length of the given ikstr instance.
 *
 * This function retrieves the length of a dynamically allocated string
 * (`ikstr`) by inspecting its metadata based on its storage type. The `ikstr`
 * instances are categorized into different types (IKSTR_8, IKSTR_16, IKSTR_32, IKSTR_64),
 * and each type has different size constraints.
 *
 * @param s The ikstr instance whose length is to be retrieved.
 *          This pointer must be valid and point to a properly allocated `ikstr`.
 * @return The length of the string stored in the `ikstr`, or 0 if the type is invalid.
 */
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

/**
 * Returns the available space for additional characters in the given ikstr instance.
 *
 * This function computes the difference between the capacity and the current
 * length of the provided `ikstr`, allowing the user to determine how many more
 * characters can be stored in the current allocation without causing reallocation.
 * The calculation is based on the type of the `ikstr` (IKSTR_8, IKSTR_16, IKSTR_32, IKSTR_64),
 * which determines the size limits of the string instance.
 *
 * @param s The ikstr instance whose available space is to be calculated.
 *          The pointer must be valid and point to a properly allocated `ikstr`.
 * @return The number of characters that can be stored in the `ikstr` without reallocation,
 *         or 0 if the type is invalid or an error occurs.
 */
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

/**
 * Sets the length of the given ikstr instance.
 *
 * This function updates the metadata for a dynamically allocated string (`ikstr`)
 * to set its length. The `ikstr` instance must have been previously allocated
 * using one of the valid size types (IKSTR_8, IKSTR_16, IKSTR_32, IKSTR_64),
 * and the appropriate type is determined by inspecting its metadata flags.
 *
 * @param s The ikstr instance whose length is to be set. This pointer must be
 *          valid and point to a properly allocated `ikstr`.
 * @param new_len The new length to assign to the `ikstr`. Behavior is undefined
 *                if this exceeds the capacity for the given `ikstr` type.
 */
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

/**
 * Increases the length of the given ikstr instance.
 *
 * This function increments the length of a dynamically allocated string (`ikstr`)
 * by the specified value. The `ikstr` types (IKSTR_8, IKSTR_16, IKSTR_32, IKSTR_64)
 * determine how the length is stored and updated. The function modifies the length
 * directly in the appropriate header based on the type of the `ikstr` instance.
 *
 * @param s The ikstr instance whose length is to be increased. This pointer must
 *          be valid and point to a properly allocated `ikstr`.
 * @param inc The amount by which to increase the length of the `ikstr`. This value
 *            is added to the current length stored in the header.
 */
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

/**
 * Returns the capacity of the given ikstr instance.
 *
 * This function retrieves the capacity of a dynamically allocated string (`ikstr`)
 * by inspecting its metadata based on the storage type. The `ikstr` instances
 * are classified into specific types (IKSTR_8, IKSTR_16, IKSTR_32, IKSTR_64),
 * and each type has different capacity constraints.
 *
 * @param s The ikstr instance whose capacity is to be retrieved.
 *          This pointer must be valid and point to a properly allocated `ikstr`.
 * @return The capacity of the string stored in the `ikstr`, or 0 if the type is invalid.
 */
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

/**
 * Sets the capacity of the given ikstr instance.
 *
 * This function updates the capacity of a dynamically allocated string
 * (`ikstr`) by modifying its metadata. The `ikstr` structure must have
 * sufficient allocated space to accommodate the new capacity. The capacity
 * is stored based on the type of the `ikstr` (IKSTR_8, IKSTR_16, IKSTR_32, IKSTR_64).
 *
 * @param s The ikstr instance for which the capacity is to be set.
 *          This pointer must be valid and point to a properly allocated `ikstr`.
 * @param new_cap The new capacity value to be assigned to the ikstr instance.
 */
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

/**
 * Creates a new ikstr instance with initial content and a specified length.
 *
 * This function allocates memory for a new dynamically allocated string (`ikstr`)
 * and initializes it with the provided content. The storage uses a header for managing
 * metadata specific to dynamically allocated strings (`ikstr`). If a valid pointer is provided
 * for the `init` parameter, the content will be copied into the new instance.
 * The length of the `ikstr` is determined by the `init_len` parameter.
 * If `init` is `IKSTR_NO_INIT`, the content will remain uninitialized.
 * If `init` is null, the allocated memory will be set to zero.
 *
 * @param init Pointer to the initial content for the string or `IKSTR_NO_INIT` for uninitialized allocation.
 *             If null, the memory will be allocated and zero-initialized.
 * @param init_len The length of the initial content or the desired length of the `ikstr`.
 *                 This must be a non-negative value.
 * @return A pointer to the newly created `ikstr` instance, or null if the allocation fails.
 */
IKSTR_API ikstr ikstr_new_len(const void* init, size_t init_len);

/**
 * Creates and returns an empty `ikstr` instance.
 *
 * This function generates a new dynamically allocated string (`ikstr`)
 * of zero length. The created string contains no characters and is
 * null-terminated.
 *
 * @return A pointer to the newly created empty `ikstr` instance, or NULL
 *         if the allocation fails.
 */
IKSTR_API ikstr ikstr_empty(void);

/**
 * Creates a new ikstr instance initialized with the specified string.
 *
 * This function creates a dynamically allocated `ikstr` object based on the
 * input string provided. If the input string is null, an empty `ikstr` will be created.
 * Internally, the function determines the appropriate type and allocates the
 * necessary memory to represent the string along with its metadata.
 *
 * @param init The null-terminated C string to initialize the `ikstr` with.
 *             If this is null, an empty `ikstr` will be created.
 * @return A newly allocated `ikstr` containing the initialized string.
 *         Returns null if memory allocation fails.
 */
IKSTR_API ikstr ikstr_new(const char *init);

/**
 * Creates a duplicate of the given `ikstr` instance.
 *
 * This function allocates a new `ikstr` instance and copies the content
 * of the provided `ikstr` into it, including its metadata and stored string.
 * The new instance is a separate allocation with the same length and content
 * as the input string.
 *
 * @param s The `ikstr` instance to be duplicated. This must be a valid
 *          instance pointing to a properly initialized `ikstr` object.
 * @return A new `ikstr` instance containing the same string content as the
 *         input `ikstr`, or NULL if memory allocation fails.
 */
IKSTR_API ikstr ikstr_dup(ikstr s);

/**
 * Frees the memory associated with a given `ikstr` instance.
 *
 * This function releases the dynamically allocated memory of an `ikstr` instance,
 * including its metadata and string content. The function ensures proper deallocation
 * by accounting for the internal header structure associated with the `ikstr`.
 *
 * @param s The `ikstr` instance to be freed. If the pointer is NULL, the function performs
 *          no operations.
 */
IKSTR_API void ikstr_free(ikstr s);

/**
 * Ensures that the given `ikstr` instance has enough capacity to hold at least
 * the specified length, expanding it if necessary.
 *
 * This function checks the current length of the `ikstr` instance and expands
 * its storage if the desired length exceeds the current length. If expansion
 * is required, new memory is allocated, and the additional space is initialized
 * to zero. The length of the `ikstr` is then updated to the specified value.
 *
 * @param s The `ikstr` instance to be grown. This pointer must be valid and point
 *          to a properly allocated `ikstr`.
 * @param len The desired length for the `ikstr` instance. If this value is less than
 *            or equal to the current length, no changes are made.
 * @return A pointer to the `ikstr` instance with the updated length, or NULL
 *         if memory allocation fails during the expansion process.
 */
IKSTR_API ikstr ikstr_grow(ikstr s, size_t len);

/**
 * Appends a given sequence of bytes to the end of an `ikstr` instance.
 *
 * This function extends the specified `ikstr` instance by concatenating a sequence
 * of bytes provided in a separate memory address. The function ensures that the
 * `ikstr` has enough capacity to hold the concatenated data. If the reallocation
 * fails or if the input `ikstr` is invalid, the function returns `NULL`.
 *
 * @param s The `ikstr` instance to which the bytes will be appended. This must be a
 *          properly allocated and valid `ikstr`.
 * @param t A pointer to the sequence of bytes to be appended. This must point to at
 *          least `len` valid bytes of memory.
 * @param len The number of bytes to append from the `t` pointer. This must be a valid
 *            non-negative value.
 * @return The modified `ikstr` instance with the concatenated content, or `NULL` if
 *         the operation fails.
 */
IKSTR_API ikstr ikstr_concat_len(ikstr s, const void* t, size_t len);

/**
 * Concatenates a null-terminated C string to an `ikstr` instance.
 *
 * This function appends the contents of the null-terminated string `t`
 * to the dynamically allocated string `ikstr` instance `s`. The original
 * `ikstr` is reallocated if necessary to accommodate the additional
 * contents. The null-terminator of `s` is preserved and updated.
 *
 * @param s The target `ikstr` instance to which the string `t` is appended.
 *          It must be a valid `ikstr` pointer.
 * @param t The null-terminated C string to append to the `ikstr`.
 *          It must be a valid pointer to a null-terminated string.
 * @return The updated `ikstr` instance containing the concatenated result.
 *         Returns `NULL` if the operation fails due to insufficient memory.
 */
IKSTR_API ikstr ikstr_concat(ikstr s, const char* t);

/**
 * Concatenates one ikstr instance to another.
 *
 * This function appends the contents of the string stored in the second
 * `ikstr` instance (`t`) to the end of the string stored in the first `ikstr`
 * instance (`s`). The resulting string's length will be the sum of the lengths
 * of the two input strings.
 *
 * Both input `ikstr` instances must be valid and properly initialized.
 *
 * @param s The base `ikstr` instance to which the second `ikstr` will be concatenated.
 *          This `ikstr` will be modified to hold the resulting concatenated string.
 *          It must have sufficient room allocated to accommodate the concatenation.
 * @param t The `ikstr` instance whose contents will be appended to `s`.
 *          This `ikstr` remains unmodified after the operation.
 * @return The modified `ikstr` instance `s` containing the concatenated string,
 *         or `NULL` if memory allocation fails or either input is invalid.
 */
IKSTR_API ikstr ikstr_concat_ikstr(ikstr s, ikstr t);

/**
 * Copies a specified number of characters from a given string into an ikstr instance.
 *
 * This function copies up to `len` characters from the source string `t` into
 * the provided `ikstr` instance `s`. If the `ikstr` instance does not have
 * sufficient capacity, it will be reallocated to accommodate the new content.
 * If reallocation fails, the function returns `NULL`. After copying, the new
 * length is set, and the resulting string is null-terminated.
 *
 * @param s The destination `ikstr` instance where the content will be copied.
 *          This pointer must be valid, or it will be reallocated if necessary.
 * @param t The source string from which characters will be copied. The pointer
 *          must be valid and contain at least `len` characters.
 * @param len The number of characters to copy from the source string `t` to `s`.
 * @return The updated `ikstr` instance containing the copied content, or `NULL`
 *         if reallocation fails.
 */
IKSTR_API ikstr ikstr_copy_len(ikstr s, const char* t, size_t len);

/**
 * Copies the content from the given C-string into the specified ikstr instance.
 *
 * This function copies the content of a null-terminated C-string `t` into the
 * dynamically allocated string `ikstr` instance `s`. If the capacity of `s` is
 * not enough to hold the content of `t`, the function resizes `s` as needed.
 * The string `s` is properly null-terminated after the operation.
 *
 * @param s The destination ikstr instance to copy into. Must be a valid `ikstr`.
 * @param t The source null-terminated C-string to copy from.
 * @return The updated ikstr instance containing the content of `t`, or `NULL` if
 *         an allocation or resizing failure occurred.
 */
IKSTR_API ikstr ikstr_copy(ikstr s, const char* t);

/**
 * Appends a formatted string to the given ikstr instance using a `va_list`.
 *
 * This function formats a string using the specified format (`fmt`) and the
 * variable argument list (`ap`), then concatenates the resulting formatted
 * string to the given `ikstr` instance. If necessary, memory is allocated to
 * accommodate the new content.
 *
 * @param s The `ikstr` instance to which the formatted string will be
 *          concatenated. This pointer must be valid and point to a properly
 *          allocated `ikstr`.
 * @param fmt The format string used to generate the text to be appended. The
 *            format specifiers in this string should match the arguments
 *            provided in the `va_list`.
 * @param ap A `va_list` containing the arguments to be used by the format
 *           specifiers in `fmt`. The caller is responsible for initializing
 *           and passing this `va_list`.
 * @return The updated `ikstr` instance that includes the formatted string
 *         appended. If an error occurs (e.g., memory allocation failure),
 *         NULL is returned.
 */
IKSTR_API ikstr ikstr_concat_vfmt(ikstr s, const char* fmt, va_list ap);

#ifdef __GNUC__
/**
 * Appends a formatted string to the given `ikstr` instance.
 *
 * This function concatenates the formatted output of the given format string and
 * additional arguments to an existing `ikstr` instance. It dynamically adjusts the
 * memory allocated to the `ikstr` to accommodate the new content if necessary.
 *
 * @param s The `ikstr` instance to which the formatted string will be appended.
 *          This pointer must be valid and point to a properly allocated `ikstr`.
 * @param fmt The format string, following the same rules as standard `printf`.
 *            This must be null-terminated and valid.
 * @param ... Additional arguments that match the format specifiers in `fmt`.
 * @return The updated `ikstr` instance with the appended formatted string, or
 *         NULL if memory allocation fails or an error occurs.
 */
IKSTR_API ikstr ikstr_concat_fmt(ikstr s, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

/**
 * Creates a new `ikstr` instance formatted according to the specified format string and arguments.
 *
 * This function allocates and initializes a new dynamically allocated string (`ikstr`)
 * by formatting the provided format string (`fmt`) with the corresponding variadic arguments.
 * It uses `vsnprintf` internally for formatting and concatenates the result into a newly
 * created `ikstr`.
 *
 * @param fmt The format string used for formatting. It must be a null-terminated C-string.
 *            The format string follows standard printf-style formatting.
 * @param ... A variadic list of arguments to be applied to the format string.
 *            These arguments must match the placeholders in the format string.
 * @return A newly allocated `ikstr` containing the formatted string on success,
 *         or `NULL` if memory allocation fails or an error occurs during formatting.
 */
IKSTR_API ikstr ikstr_new_fmt(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
#else
/**
 * Appends a formatted string to the given `ikstr` instance.
 *
 * This function concatenates the formatted output of the given format string and
 * additional arguments to an existing `ikstr` instance. It dynamically adjusts the
 * memory allocated to the `ikstr` to accommodate the new content if necessary.
 *
 * @param s The `ikstr` instance to which the formatted string will be appended.
 *          This pointer must be valid and point to a properly allocated `ikstr`.
 * @param fmt The format string, following the same rules as standard `printf`.
 *            This must be null-terminated and valid.
 * @param ... Additional arguments that match the format specifiers in `fmt`.
 * @return The updated `ikstr` instance with the appended formatted string, or
 *         NULL if memory allocation fails or an error occurs.
 */
IKSTR_API ikstr ikstr_concat_fmt(ikstr s, const char* fmt, ...);

/**
 * Creates a new `ikstr` instance formatted according to the specified format string and arguments.
 *
 * This function allocates and initializes a new dynamically allocated string (`ikstr`)
 * by formatting the provided format string (`fmt`) with the corresponding variadic arguments.
 * It uses `vsnprintf` internally for formatting and concatenates the result into a newly
 * created `ikstr`.
 *
 * @param fmt The format string used for formatting. It must be a null-terminated C-string.
 *            The format string follows standard printf-style formatting.
 * @param ... A variadic list of arguments to be applied to the format string.
 *            These arguments must match the placeholders in the format string.
 * @return A newly allocated `ikstr` containing the formatted string on success,
 *         or `NULL` if memory allocation fails or an error occurs during formatting.
 */
IKSTR_API ikstr ikstr_new_fmt(const char* fmt, ...);
#endif

/**
 * Trims characters from both ends of the given ikstr instance.
 *
 * This function removes all characters present in the `char_set` from
 * the beginning and end of the provided `ikstr` string. The operation
 * adjusts the string in place, modifying its contents and metadata.
 *
 * @param s The ikstr instance to be trimmed. This pointer must be valid
 *          and point to a dynamically allocated `ikstr`.
 * @param char_set A null-terminated string containing characters to be
 *                 removed from the beginning and end of `s`. This must be
 *                 a valid C string.
 * @return The trimmed ikstr instance. If no trimming is needed, the
 *         original instance is returned unchanged.
 */
IKSTR_API ikstr ikstr_trim(ikstr s, const char* char_set);

/**
 * Modifies the given ikstr instance to contain only the substring defined
 * by the specified start and end indexes.
 *
 * This function adjusts the content of the given `ikstr` in-place, keeping
 * only the characters within the range [start, end]. Negative indices for
 * start or end are treated as offsets from the end of the string.
 * The length of the string is updated accordingly, and the resulting string
 * is null-terminated. If the range is invalid or results in an empty range,
 * the `ikstr` is set to an empty string.
 *
 * @param s The `ikstr` instance to be modified. This pointer must point
 *          to a valid, dynamically allocated ikstr string.
 * @param start The starting index of the range. Can be negative to indicate
 *              an offset from the end of the string.
 * @param end The ending index of the range. Can be negative to indicate
 *            an offset from the end of the string.
 */
IKSTR_API void ikstr_range(ikstr s, ssize_t start, ssize_t end);

/**
 * Compares two ikstr instances lexicographically.
 *
 * This function performs a comparison between two dynamically allocated
 * strings (`ikstr`). The comparison is carried out lexicographically,
 * meaning that the strings are compared character by character in order.
 * If the strings are identical up to the length of the shorter one,
 * their lengths are used to determine the result.
 *
 * @param s1 The first ikstr instance to compare. This pointer must be valid
 *           and point to a properly allocated `ikstr`.
 * @param s2 The second ikstr instance to compare. This pointer must also be
 *           valid and point to a properly allocated `ikstr`.
 * @return A negative value if `s1` is lexicographically less than `s2`,
 *         a positive value if `s1` is lexicographically greater than `s2`,
 *         or 0 if `s1` and `s2` are equal.
 */
IKSTR_API int ikstr_cmp(ikstr s1, ikstr s2);

/**
 * Splits a string into parts based on a specified separator.
 *
 * This function takes a string `s` of length `len` and splits it into parts
 * wherever the specified separator `sep` (of length `sep_len`) appears. The
 * function dynamically allocates memory to store the resulting parts as an
 * array of `ikstr` instances. The number of parts found is stored in the
 * provided `count` parameter. If the separator is not found or invalid input
 * is provided, the function may return NULL.
 *
 * @param s The input string to split. Must not be NULL.
 * @param len The length of the input string `s`. Must be greater than 0.
 * @param sep The separator string used to split `s`. Must not be NULL.
 * @param sep_len The length of the separator string `sep`. Must be greater than 0.
 * @param count A pointer to an integer where the number of resulting parts
 *              will be stored. Must be a valid pointer.
* @return An array of `ikstr` instances containing the substrings, or `NULL` if an error occurs.
 *         The caller is responsible for freeing each `ikstr` in the array, as well as the array
 *         itself, using `ikstr_free_split_res`.
 */
IKSTR_API ikstr *ikstr_split_len(const char *s, ssize_t len, const char *sep, int sep_len, int *count);

/**
 * Splits the given string into an array of `ikstr` instances based on a specified separator.
 *
 * This function divides the input string `s` into substrings, using the delimiter `sep`,
 * and allocates an array of `ikstr` instances to hold the resulting substrings. The
 * number of substrings is stored in the variable pointed to by `count`.
 * If the input string or separator is invalid, the function will return `NULL`, and `count` will be set to 0.
 *
 * @param s The input string to be split. Must be a valid null-terminated string.
 * @param sep The separator string used to split `s`. Must be a valid null-terminated string.
 * @param count Pointer to an integer where the number of resulting substrings will be stored.
 *              On error, this value will be set to 0.
 * @return An array of `ikstr` instances containing the substrings, or `NULL` if an error occurs.
 *         The caller is responsible for freeing each `ikstr` in the array, as well as the array
 *         itself, using `ikstr_free_split_res`.
 */
IKSTR_API ikstr *ikstr_split(const char *s, const char *sep, int *count);

/**
 * Frees the memory allocated for an array of `ikstr` instances.
 *
 * This function is used to deallocate the memory for a dynamically allocated
 * array of strings (`ikstr`) and their contents. Each string in the array is
 * individually freed, followed by the array itself. It is important to ensure
 * that the `tokens` pointer is valid and points to a properly allocated array
 * of `ikstr`.
 *
 * @param tokens The array of `ikstr` instances to be freed. If this pointer is NULL, the function does nothing.
 * @param count The number of entries in the `tokens` array to be freed.
 */
IKSTR_API void ikstr_free_split_res(ikstr *tokens, int count);

/**
 * Joins multiple strings into a single `ikstr` instance, separating them
 * using the specified delimiter.
 *
 * This function iterates through the given array of strings (`argv`),
 * concatenates them into a new dynamically allocated `ikstr`, and inserts
 * the specified separator string (`sep`) between the concatenated strings.
 *
 * @param argc The number of strings to concatenate. This must be greater than or equal to 0.
 * @param argv An array of C strings to be joined. The array must contain `argc` valid null-terminated
 *             strings. Each string pointer in the array must be valid.
 * @param sep  The separator string to use between concatenated strings. This can be empty or null.
 * @return A newly allocated `ikstr` instance containing the concatenated result,
 *         or NULL if memory allocation fails.
 */
IKSTR_API ikstr ikstr_join(int argc, char** argv, const char* sep);

/**
 * Joins multiple `ikstr` strings with a specified separator.
 *
 * This function combines an array of `ikstr` strings into a single `ikstr`,
 * inserting the provided separator between each string. If `argc` is 0,
 * the function will return an empty `ikstr`. If `sep` is `NULL`, no separator
 * will be used between the strings.
 *
 * @param argc The number of `ikstr` strings in the `argv` array.
 *             Must be a non-negative integer.
 * @param argv An array of `ikstr` strings to be joined.
 *             Each element must be a valid dynamically allocated `ikstr`.
 * @param sep  The separator to be inserted between strings.
 *             If `NULL`, no separator is used.
 * @return A newly allocated `ikstr` containing the joined strings.
 *         Returns an empty `ikstr` if `argc` is 0. Returns `NULL` on memory
 *         allocation failure.
 */
IKSTR_API ikstr ikstr_join_ikstr(int argc, ikstr *argv, const char* sep);


/**
 * Ensures that the given `ikstr` instance has enough available space to accommodate
 * additional data of the specified length. If the current capacity is insufficient,
 * the function reallocates the `ikstr` with a larger capacity.
 *
 * This method is used to dynamically expand the storage space associated with
 * an `ikstr`, preserving existing data within the string while allowing for efficient
 * future operations that require more space.
 *
 * @param s The `ikstr` instance to modify. This must be a valid `ikstr` pointer.
 * @param addlen The additional length (in bytes) that the `ikstr` needs to accommodate.
 *               This value will be added to the current length of the string.
 * @return A pointer to the updated `ikstr` instance, with sufficient capacity
 *         for the additional space. If allocation fails or if a size overflow
 *         occurs during calculations, the function returns `NULL`.
 */
IKSTR_API ikstr ikstr_make_room_for(ikstr s, size_t addlen);

#endif //IKSTR_IKSTR_H