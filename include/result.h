#ifndef BETTERR_RESULT_H
#define BETTERR_RESULT_H

#include "betterr.h"

#include <stddef.h>

#define DEFINE_RESULT(T, Name)                      \
    typedef struct                                  \
    {                                               \
        T value;                                    \
        Betterr *error;                             \
    } Name;                                         \
    static inline Name Name##_OK(T val)             \
    {                                               \
        return (Name){.value = val, .error = NULL}; \
    }                                               \
    static inline Name Name##_ERR(Betterr *err)     \
    {                                               \
        return (Name){.error = err};                \
    }

#define TRY(expr, ReturnType)                             \
    ({                                                    \
        __auto_type _r = (expr);                          \
        if (_r.error)                                     \
            return ReturnType##_ERR(PROPAGATE(_r.error)); \
        _r.value;                                         \
    })

#define _DEFER_CONCAT(a, b) a##b
#define _DEFER_VAR(line) _DEFER_CONCAT(_defer_, line)

#define DEFER(func, var) __auto_type _DEFER_VAR(__LINE__) __attribute__((cleanup(func))) = (var)

#endif