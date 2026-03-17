#ifndef BETTERR_H
#define BETTERR_H

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
} Betterr;

Betterr *betterr_new(const Domain *domain, uint32_t code, Location loc, const char *fmt, ...);
Betterr *betterr_propagate(Betterr *cause, Location loc);
Betterr *betterr_from(Betterr *cause, const Domain *domain, uint32_t code, Location loc, const char *fmt, ...);
void betterr_free(Betterr **err);
void betterr_print(const Betterr *err);

#define ERRLOC_HERE ((Location){__FILE__, __func__, __LINE__})

#define BETTERR(domain, code, fmt, ...) betterr_new(domain, code, ERRLOC_HERE, fmt, ##__VA_ARGS__)
#define PROPAGATE(cause) betterr_propagate(cause, ERRLOC_HERE)
#define BETTERR_FROM(cause, domain, code, fmt, ...) betterr_from(cause, domain, code, ERRLOC_HERE, fmt, ##__VA_ARGS__)

#define DEFINE_DOMAIN(var, name) const Domain var = {name}

#endif