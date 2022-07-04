#ifndef ULCTL_LIGHT_H
#define ULCTL_LIGHT_H

#include "error.h"

#include <libudev.h>
#include <stdbool.h>

struct ulctl_light {
    struct udev_device *device;
    const char *name;
    const char *subsystem;
    unsigned int max_brightness;

    unsigned int brightness;
};

ulctl_error_t ulctl_light_get(struct udev *udev, struct ulctl_light *light, const char *name);
ulctl_error_t ulctl_light_get_default(struct udev *udev, struct ulctl_light *light);

void ulctl_light_destroy(struct ulctl_light *light);

ulctl_error_t ulctl_light_read(struct ulctl_light *light);
ulctl_error_t ulctl_light_write(const struct ulctl_light *light);

void ulctl_light_print(const struct ulctl_light *light, bool machine);

#endif
