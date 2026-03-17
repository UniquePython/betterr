#include "betterr.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

Betterr *betterr_new(const Domain *domain, uint32_t code, Location loc, const char *fmt, ...)
{
    Betterr *err = malloc(sizeof(Betterr));
    if (!err)
        abort();

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    err->message = malloc(len + 1);
    if (!err->message)
        abort();

    vsnprintf(err->message, len + 1, fmt, args_copy);
    va_end(args_copy);

    err->domain = domain;
    err->code = code;
    err->location = loc;
    err->cause = NULL;

    return err;
}

Betterr *betterr_propagate(Betterr *cause, Location loc)
{
    Betterr *err = malloc(sizeof(Betterr));
    if (!err)
        abort();

    err->domain = NULL;
    err->code = 0;
    err->message = NULL;
    err->location = loc;
    err->cause = cause;

    return err;
}

Betterr *betterr_from(Betterr *cause, const Domain *domain, uint32_t code, Location loc, const char *fmt, ...)
{
    Betterr *err = malloc(sizeof(Betterr));
    if (!err)
        abort();

    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    err->message = malloc(len + 1);
    if (!err->message)
        abort();

    vsnprintf(err->message, len + 1, fmt, args_copy);
    va_end(args_copy);

    err->domain = domain;
    err->code = code;
    err->location = loc;
    err->cause = cause;

    return err;
}

void betterr_free(Betterr **err)
{
    if (!err || !*err)
        return;

    Betterr *current = *err;
    while (current)
    {
        Betterr *next = current->cause;
        free(current->message);
        free(current);
        current = next;
    }

    *err = NULL;
}

void betterr_print(const Betterr *err)
{
    if (!err)
        return;

    const Betterr *current = err;
    int depth = 0;

    while (current)
    {
        for (int i = 0; i < depth; i++)
            fprintf(stderr, "  ");

        if (current->message)
            fprintf(stderr, "%s:%u in %s() (%s:%d): %s\n",
                    current->domain ? current->domain->name : "unknown",
                    current->code,
                    current->location.function,
                    current->location.file,
                    current->location.line,
                    current->message);
        else
            fprintf(stderr, "propagated through %s() (%s:%d)\n",
                    current->location.function,
                    current->location.file,
                    current->location.line);

        if (current->cause)
            fprintf(stderr, "%*scaused by: \n", depth * 2, "");

        current = current->cause;
        depth++;
    }
}