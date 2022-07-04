#include "error.h"

#include <string.h>

ulctl_error_t ulctl_ok(void) {
    ulctl_error_t error;
    memset(&error, 0, sizeof(ulctl_error_t));

    return error;
}

ulctl_error_t ulctl_error(const char *e) {
    ulctl_error_t error = {
        .is_error = true,
        .error = e,
    };

    return error;
}
