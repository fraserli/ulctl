#include "light.h"

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ulctl_error_t ulctl_light_get(struct udev *udev, struct ulctl_light *light, const char *name) {
    struct udev_device *device;
    device = udev_device_new_from_subsystem_sysname(udev, "backlight", name);
    if (!device) {
        device = udev_device_new_from_subsystem_sysname(udev, "leds", name);
        if (!device) {
            return ulctl_error("unable to read device");
        }
    }

    light->device = device;
    light->name = udev_device_get_sysname(device);
    light->subsystem = udev_device_get_subsystem(device);

    ulctl_error_t error = ulctl_light_read(light);
    if (error.is_error) {
        udev_device_unref(device);
        return error;
    }

    return ulctl_ok();
}

ulctl_error_t ulctl_light_get_default(struct udev *udev, struct ulctl_light *light) {
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    if (!enumerate) {
        return ulctl_error("failed to create udev enumerate object");
    }

    if (udev_enumerate_add_match_subsystem(enumerate, "backlight") < 0) {
        udev_enumerate_unref(enumerate);
        return ulctl_error("failed to match backlight subsystem");
    }

    if (udev_enumerate_scan_devices(enumerate) < 0) {
        udev_enumerate_unref(enumerate);
        return ulctl_error("failed to scan devices");
    }

    struct udev_list_entry *list_entry = udev_enumerate_get_list_entry(enumerate);

    if (!list_entry) {
        return ulctl_error("unable to find light device");
    }

    struct udev_device *device =
        udev_device_new_from_syspath(udev, udev_list_entry_get_name(list_entry));
    if (!device) {
        return ulctl_error("unable to read device");
    }

    light->device = device;
    light->name = udev_device_get_sysname(device);
    light->subsystem = udev_device_get_subsystem(device);

    ulctl_error_t error = ulctl_light_read(light);
    if (error.is_error) {
        udev_device_unref(device);
        return error;
    }

    udev_enumerate_unref(enumerate);

    return ulctl_ok();
}

void ulctl_light_destroy(struct ulctl_light *light) {
    udev_device_unref((struct udev_device *)light->device);
}

ulctl_error_t ulctl_light_read(struct ulctl_light *light) {
    const char *brightness_s = udev_device_get_sysattr_value(light->device, "brightness");
    if (!brightness_s) {
        return ulctl_error("failed to get brightness");
    }

    const char *max_brightness_s = udev_device_get_sysattr_value(light->device, "max_brightness");
    if (!max_brightness_s) {
        return ulctl_error("failed to get max_brightness");
    }

    light->brightness = strtoul(brightness_s, NULL, 10);
    light->max_brightness = strtoul(max_brightness_s, NULL, 10);

    return ulctl_ok();
}

ulctl_error_t ulctl_light_write(const struct ulctl_light *light) {
    char brightness_s[128];
    snprintf(brightness_s, 128, "%i", light->brightness);
    if (udev_device_set_sysattr_value(light->device, "brightness", brightness_s) < 0) {
        return ulctl_error("unable to set brightness");
    }
    return ulctl_ok();
}

void ulctl_light_print(const struct ulctl_light *light, bool machine) {
    if (machine) {
        printf("%s,%s,%.2f%%,%i,%i\n", light->name, light->subsystem,
               ((double)light->brightness / (double)light->max_brightness) * 100.0,
               light->brightness, light->max_brightness);
    } else {
        printf("Device \"%s\" (%s):\n"
               "    Current brightness: %i (%.2f%%)\n"
               "    Max brightness: %i\n",
               light->name, light->subsystem, light->brightness,
               ((double)light->brightness / (double)light->max_brightness) * 100.0,
               light->max_brightness);
    }
}
