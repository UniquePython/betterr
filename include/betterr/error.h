#ifndef BETTERR_ERROR_H
#define BETTERR_ERROR_H

#include <stdint.h>

typedef struct
{
    const char *name;
} Domain;

typedef struct
{
    const char *file;
    const char *function;
    int line;
} Location;

typedef struct s_error
{
    const Domain *domain;
    uint32_t code;
    char *message;
    Location location;
    struct s_error *cause;
} Error;

Error *error_new(const Domain *domain, uint32_t code, Location loc, const char *fmt, ...);
Error *error_propagate(Error *cause, Location loc);
Error *error_from(Error *cause, const Domain *domain, uint32_t code, Location loc, const char *fmt, ...);
void error_free(Error **err);
void error_print(const Error *err);

#define ERRLOC_HERE ((Location){__FILE__, __func__, __LINE__})

#define ERR(domain, code, fmt, ...) error_new(domain, code, ERRLOC_HERE, fmt, ##__VA_ARGS__)
#define PROPAGATE(cause) error_propagate(cause, ERRLOC_HERE)
#define ERR_FROM(cause, domain, code, fmt, ...) error_from(cause, domain, code, ERRLOC_HERE, fmt, ##__VA_ARGS__)

#define DEFINE_DOMAIN(var, name) const Domain var = {name}

#endif