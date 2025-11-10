# ikstr

A tiny C library providing a heap-allocated, length-prefixed string type with convenient creation, duplication, and destruction APIs. It aims to offer safer and more ergonomic handling than raw C strings while staying lightweight and dependency-free.

## Features

- Contiguous, null-terminated buffer compatible with C string functions
- O(1) access to length and capacity stored in a small header
- Multiple header widths are selected automatically based on size
- Simple API: create, duplicate, query length/capacity, and free
- Custom allocator hooks (via `ikstr_alloc.h`) for integration with your memory management

## Getting Started

### Requirements
- C11-compatible compiler
- CMake 3.15+ (recommended)

### Installation
If you're using Linux, you can install `ikstr` using apt. First, you have to add my GitHub Pages as a source for apt:
```bash
echo "deb [trusted=yes] https://ikryxx.github.io/ikstr/debian ./" | sudo tee /etc/apt/sources.list.d/ikstr.list
```
after that, you will run:
```bash
apt clean && apt update && apt install ikstr
```
this will install ikstr and all its dependencies.

### Build
To build the library manually, run the following commands (You may want to edit the CMakeLists.txt to remove the testing executable)
```bash 
cd <path/to/ikstr>
cmake -S . -B build 
cmake --build build
```

## Usage

### Create and free

```c
#include <ikstr.h>
#include <stdio.h>

int main(void) {
    ikstr s = ikstr_new("Hello, ikstr!");
    // You can now use s as a C string
    printf("%s\n", s);
    ikstr_free(s);
    return 0;
}
```

### API overview
detailed examples can be found in `test/main.c`.
- Creation:
  - `ikstr ikstr_new(const char* init);`
  - `ikstr ikstr_new_len(const void* init, size_t len);`
  - `ikstr ikstr_empty(void);`
  - `ikstr ikstr_new_fmt(const char* fmt, ...);`
- Copy:
  - `ikstr ikstr_dup(ikstr s);`
- Query:
  - `size_t ikstr_len(ikstr s);`
  - `size_t ikstr_cap(ikstr s);`
  - `size_t ikstr_avail(ikstr s);`
- Mutate metadata:
  - `void ikstr_set_len(ikstr s, size_t new_len);`
  - `void ikstr_inc_len(ikstr s, size_t inc);`
- Append/modify data:
  - `ikstr ikstr_concat(ikstr s, const char* t);`
  - `ikstr ikstr_concat_len(ikstr s, const void* t, size_t len);`
  - `ikstr ikstr_concat_ikstr(ikstr s, ikstr t);`
  - `ikstr ikstr_concat_fmt(ikstr s, const char* fmt, ...);`
  - `ikstr ikstr_copy(ikstr s, const char* t);`
  - `ikstr ikstr_copy_len(ikstr s, const char* t, size_t len);`
  - `ikstr ikstr_grow(ikstr s, size_t len);`
- Transform:
  - `ikstr ikstr_trim(ikstr s, const char* char_set);`
  - `void ikstr_range(ikstr s, ssize_t start, ssize_t end);`
- Compare:
  - `int ikstr_cmp(ikstr s1, ikstr s2);`
- Split/join:
  - `ikstr* ikstr_split(const char* s, const char* sep, int* count);`
  - `ikstr* ikstr_split_len(const char* s, ssize_t len, const char* sep, int sep_len, int* count);`
  - `void ikstr_free_split_res(ikstr* tokens, int count);`
  - `ikstr ikstr_join(int argc, char** argv, const char* sep);`
  - `ikstr ikstr_join_ikstr(int argc, ikstr* argv, const char* sep);`
- Low-level:
  - `ikstr ikstr_make_room_for(ikstr s, size_t addlen);`
- Destroy:
  - `void ikstr_free(ikstr s);`

Notes:
- `ikstr` is a `char*` pointing to a null-terminated buffer; you can pass it to standard C functions that expect a C string.
- Length and capacity are tracked in a hidden header preceding the buffer; always use `ikstr_len` and `ikstr_cap` to access them and `ikstr_free` to free the string.
- When manually changing length via `ikstr_set_len`/`ikstr_inc_len`, ensure the buffer remains null-terminated and within capacity.

## Custom Allocation

If you need custom allocators (e.g., for tracking or arenas), provide implementations for the allocation hooks declared in `ikstr_alloc.h` (e.g., `iks_malloc`).

## Contributing
I am amenable to contributions and feedback. Please open an issue or pull request if you have any suggestions or feedback, however, please follow these rules:
- Keep the API minimal and consistent.
- Add tests for new behavior.
- Follow the existing code style.

## License

MIT. See LICENSE if provided in the repository; otherwise the project is intended to be used permissively.
