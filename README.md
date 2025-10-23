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
detailed documentation and examples will be added in the future.
- Creation:
  - `ikstr ikstr_new(const char* init);`
  - `ikstr ikstr_new_len(const void* init, size_t len)`
  - `ikstr ikstr_empty(void);`
- Copy:
    - `ikstr ikstr_dup(ikstr s);`
- Query:
    - `size_t ikstr_len(ikstr s);`
    - `size_t ikstr_cap(ikstr s);`
    - `size_t ikstr_avail(ikstr s);`
- Mutate metadata:
    - `void ikstr_set_len(ikstr s, size_t new_len);`
    - `void ikstr_inc_len(ikstr s, size_t inc);`
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
