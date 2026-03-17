# betterr

C error handling that doesn't make you want to quit. Typed results, rich error chains, automatic cleanup, and full source location tracking.

---

## Installation

### Requirements
- GCC 4.9+ or any modern Clang (uses __auto_type, __attribute__((cleanup)), statement expressions — MSVC is not supported)
- CMake 3.15+

### Building
```bash
git clone https://github.com/UniquePython/betterr.git
cd betterr
cmake -B build
cmake --build build
```

### Linking against betterr

In your `CMakeLists.txt`:
```cmake
add_subdirectory(betterr)
target_link_libraries(your_target PRIVATE betterr)
```

Then include:
```c
#include "betterr.h"
#include "result.h"
```

---

## Usage

### 1. Define a domain

Every library defines its own domain. Identity is pointer-based -- no collision possible between libraries.
```c
// mylib.c
DEFINE_DOMAIN(MY_DOMAIN, "mylib");

// mylib.h
extern const Domain MY_DOMAIN;
```

### 2. Define result types
```c
DEFINE_RESULT(int,   INT)
DEFINE_RESULT(char*, STR)
```

### 3. Return errors
```c
INT parse_int(const char *s)
{
    if (!s)
        return INT_ERR(BETTERR(&MY_DOMAIN, ERR_INVALID, "input is NULL"));
    return INT_OK(42);
}
```

### 4. Propagate with TRY
```c
INT load_config(const char *path)
{
    int val = TRY(parse_int(NULL), INT);
    return INT_OK(val);
}
```

`TRY` adds a location-only node to the chain and returns early on failure. No message needed -- the origin error already has one.

### 5. Wrap with context

When you have something meaningful to add:
```c
INT init(const char *path)
{
    INT r = load_config(path);
    if (r.error)
        return INT_ERR(BETTERR_FROM(r.error, &MY_DOMAIN, ERR_INIT, "failed to initialize"));
    return INT_OK(r.value);
}
```

### 6. Handle at the top
```c
INT r = init("config.cfg");
if (r.error)
{
    betterr_print(r.error);
    betterr_free(&r.error);
}
```

### 7. Automatic cleanup with DEFER
```c
void fclose_defer(FILE **f) { if (*f) fclose(*f); }
void free_defer(void **p)   { if (*p) { free(*p); *p = NULL; } }

INT read_file(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return INT_ERR(BETTERR(&MY_DOMAIN, ERR_IO, "failed to open '%s'", path));
    DEFER(fclose_defer, f);

    char *buf = malloc(1024);
    if (!buf) return INT_ERR(BETTERR(&MY_DOMAIN, ERR_OOM, "out of memory"));
    DEFER(free_defer, buf);

    // f and buf are freed automatically on any return path
    return INT_OK(0);
}
```

### Output
```
propagated through load_config() (src/main.c:61)
caused by:
  mylib:1 in parse_int() (src/main.c:55): input is NULL
```

---

## API Reference

### Types

#### `Domain`
```c
typedef struct { const char *name; } Domain;
```
Identifies which library an error belongs to. Identity is pointer equality -- never compare by name.

#### `Betterr`
```c
typedef struct Betterr {
    const Domain *domain;
    uint32_t      code;
    char         *message;
    Location      location;
    struct Betterr *cause;
} Betterr;
```
Heap allocated. Owns its cause chain. Never free a `Betterr` that has been passed as cause to another -- ownership is transferred.

---

### Macros

#### `DEFINE_DOMAIN(var, name)`
Defines a domain variable. Always place in a `.c` file, never a header.
```c
DEFINE_DOMAIN(MY_DOMAIN, "mylib");
```

#### `DEFINE_RESULT(T, Name)`
Generates a typed Result struct with `Name_OK` and `Name_ERR` constructors.
```c
DEFINE_RESULT(int, INT)
// produces: INT, INT_OK(val), INT_ERR(err)
```

#### `BETTERR(domain, code, fmt, ...)`
Creates a new error at the current source location.
```c
BETTERR(&MY_DOMAIN, 1, "failed to open '%s'", path);
```

#### `BETTERR_FROM(cause, domain, code, fmt, ...)`
Creates a new error that wraps an existing one. Takes ownership of `cause`.
```c
BETTERR_FROM(inner, &MY_DOMAIN, 2, "operation failed");
```

#### `PROPAGATE(cause)`
Creates a location-only node in the chain. No message. Takes ownership of `cause`.
```c
PROPAGATE(inner);
```

#### `TRY(expr, ReturnType)`
Evaluates a Result expression. On failure, propagates with location and returns early.
```c
int val = TRY(parse_int(s), INT);
```

#### `DEFER(func, var)`
Registers a cleanup function to run when the current scope exits.
```c
DEFER(fclose_defer, f);
```
Cleanup functions receive a pointer to the variable: `void fn(T **var)`.

---

### Functions

#### `betterr_print(const Betterr *err)`
Prints the full error chain to stderr. Top-down order.

#### `betterr_free(Betterr **err)`
Frees the entire error chain and nulls the pointer. Safe to call on NULL.

---

## License

MIT