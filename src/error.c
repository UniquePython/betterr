#include "betterr/error.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

Error *error_new(const Domain *domain, uint32_t code, Location loc, const char *fmt, ...)
{
    Error *err = malloc(sizeof(Error));
    if (!err)
        return NULL;

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    err->message = malloc(len + 1);
    if (!err->message)
    {
        va_end(args_copy);
        free(err);
        return NULL;
    }

    vsnprintf(err->message, len + 1, fmt, args_copy);
    va_end(args_copy);

    err->domain = domain;
    err->code = code;
    err->location = loc;
    err->cause = NULL;

    return err;
}

void error_free(Error **err)
{
    if (!err || !*err)
        return;

    Error *current = *err;
    while (current)
    {
        Error *next = current->cause;
        free(current->message);
        free(current);
        current = next;
    }

    *err = NULL;
}