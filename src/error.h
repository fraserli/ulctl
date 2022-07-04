#ifndef ULCTL_ERROR_H
#define ULCTL_ERROR_H

#include <stdbool.h>

typedef struct ulctl_error {
    bool is_error;
    const char *error;
} ulctl_error_t;

ulctl_error_t ulctl_ok(void);
ulctl_error_t ulctl_error(const char *e);

#endif
