#include "error.h"
#include "light.h"

#include <libudev.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct args {
    unsigned int h : 1;
    unsigned int m : 1;
    unsigned int s : 1;
    unsigned int v : 1;
    unsigned int q : 1;
    const char *d;

    int argc;
    char **argv;
};

static int info(const struct args *args);
static int list(const struct args *args);
static int set(const struct args *args);
static int inc(const struct args *args, bool dec);

int main(int argc, char **argv) {
    struct args args;
    memset(&args, 0, sizeof(struct args));

    int opt;
    while ((opt = getopt(argc, argv, "d:hmsvq")) != -1) {
        switch (opt) {
        case 'd':
            args.d = optarg;
            break;
        case 'h':
            args.h = 1;
            break;
        case 'm':
            args.m = 1;
            break;
        case 's':
            args.s = 1;
            break;
        case 'v':
            args.v = 1;
            break;
        case 'q':
            args.q = 1;
            break;
        case '?':
            return 64;
        }
    }

    if (args.h) {
        printf("Usage: ulctl [<options>...] <command> [<command_args>...]\n\n"
               "Commands:\n"
               "    info        Show device information (default).\n"
               "    list        List all available devices.\n"
               "    set VALUE   Set device brightness.\n"
               "    inc VALUE   Increase brightness.\n"
               "    dec VALUE   Decrease brightness.\n\n"
               "Options:\n"
               "    -d DEVICE   Set device (eg. \"intel_backlight\").\n"
               "    -h          Show usage information and exit.\n"
               "    -m          Enable machine readable output.\n"
               "    -s          Use specific values instead of percentage.\n"
               "    -v          Show version information and exit.\n"
               "    -q          Suppress output of set, inc, dec commands.\n");
        return 0;
    } else if (args.v) {
        printf("ulctl v%s\n", PROJECT_VERSION);
        return 0;
    }

    if (optind >= argc) {
        args.argc = argc - optind;
        args.argv = argv + optind;
        return info(&args);
        fprintf(stderr, "No command provided\n");
        return 64;
    }

    args.argc = argc - optind - 1;
    args.argv = argv + optind + 1;

    if (strcmp("info", argv[optind]) == 0) {
        return info(&args);
    } else if (strcmp("list", argv[optind]) == 0) {
        return list(&args);
    } else if (strcmp("set", argv[optind]) == 0) {
        return set(&args);
    } else if (strcmp("inc", argv[optind]) == 0) {
        return inc(&args, false);
    } else if (strcmp("dec", argv[optind]) == 0) {
        return inc(&args, true);
    } else {
        fprintf(stderr, "Invalid command \"%s\"\n", argv[optind]);
        return 64;
    }
}

static int info(const struct args *args) {
    struct udev *udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Failed to create udev context\n");
        return 1;
    }

    struct ulctl_light light;
    if (args->d) {
        ulctl_error_t error = ulctl_light_get(udev, &light, args->d);
        if (error.is_error) {
            fprintf(stderr, "Failed to get light \"%s\": %s\n", args->d, error.error);
            udev_unref(udev);
            return 1;
        }
    } else {
        ulctl_error_t error = ulctl_light_get_default(udev, &light);
        if (error.is_error) {
            fprintf(stderr, "Failed to get light: %s\n", error.error);
            udev_unref(udev);
            return 1;
        }
    }

    ulctl_light_print(&light, args->m);

    ulctl_light_destroy(&light);
    udev_unref(udev);

    return 0;
}

static int list(const struct args *args) {
    struct udev *udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Failed to create udev context\n");
        return 1;
    }

    size_t count = 0;
    struct ulctl_light *lights;
    ulctl_error_t error = ulctl_light_list(udev, &lights, &count);
    if (error.is_error) {
        fprintf(stderr, "Failed to list lights: %s\n", error.error);
        udev_unref(udev);
        return 1;
    }

    for (size_t i = 0; i < count; i++) {
        ulctl_light_print(&lights[i], args->m);
        if (i < count - 1) {
            putchar('\n');
        }
        ulctl_light_destroy(&lights[i]);
    }

    free(lights);
    udev_unref(udev);

    return 0;
}

static int set(const struct args *args) {
    if (args->argc < 1) {
        fprintf(stderr, "No value provided\n");
        return 64;
    }

    char *endptr;
    double value = strtod(args->argv[0], &endptr);
    if (endptr != args->argv[0] + strlen(args->argv[0])) {
        fprintf(stderr, "Invalid value: \"%s\"\n", args->argv[0]);
        return 64;
    }

    struct udev *udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Failed to create udev context\n");
        return 1;
    }

    struct ulctl_light light;
    if (args->d) {
        ulctl_error_t error = ulctl_light_get(udev, &light, args->d);
        if (error.is_error) {
            fprintf(stderr, "Failed to get light \"%s\": %s\n", args->d, error.error);
            udev_unref(udev);
            return 1;
        }
    } else {
        ulctl_error_t error = ulctl_light_get_default(udev, &light);
        if (error.is_error) {
            fprintf(stderr, "Failed to get light: %s\n", error.error);
            udev_unref(udev);
            return 1;
        }
    }

    if (value < 0 || value > (args->s ? (double)light.max_brightness : 100.0)) {
        fprintf(stderr, "Brightness value \"%s\" is out of range (0-%i)\n", args->argv[0],
                args->s ? light.max_brightness : 100);
        ulctl_light_destroy(&light);
        udev_unref(udev);
        return 64;
    }

    if (args->s) {
        light.brightness = (unsigned int)round(value);
    } else {
        light.brightness = (unsigned int)round(light.max_brightness * (value / 100.0));
    }

    ulctl_error_t error = ulctl_light_write(&light);
    if (error.is_error) {
        fprintf(stderr, "Failed to write to device %s\n", light.name);
        ulctl_light_destroy(&light);
        udev_unref(udev);
        return 1;
    }

    if (!args->q) {
        ulctl_light_print(&light, args->m);
    }

    ulctl_light_destroy(&light);
    udev_unref(udev);

    return 0;
}

static int inc(const struct args *args, bool dec) {
    if (args->argc < 1) {
        fprintf(stderr, "No value provided\n");
        return 64;
    }

    char *endptr;
    double value = strtod(args->argv[0], &endptr);
    if (endptr != args->argv[0] + strlen(args->argv[0])) {
        fprintf(stderr, "Invalid value: \"%s\"\n", args->argv[0]);
        return 64;
    }

    struct udev *udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Failed to create udev context\n");
        return 1;
    }

    struct ulctl_light light;
    if (args->d) {
        ulctl_error_t error = ulctl_light_get(udev, &light, args->d);
        if (error.is_error) {
            fprintf(stderr, "Failed to get light \"%s\": %s\n", args->d, error.error);
            udev_unref(udev);
            return 1;
        }
    } else {
        ulctl_error_t error = ulctl_light_get_default(udev, &light);
        if (error.is_error) {
            fprintf(stderr, "Failed to get light: %s\n", error.error);
            udev_unref(udev);
            return 1;
        }
    }

    if (args->s) {
        if (dec && value >= light.brightness) {
            light.brightness = 0;
        } else if (dec) {
            light.brightness -= round(value);
        } else if (value + light.brightness >= light.max_brightness) {
            light.brightness = light.max_brightness;
        } else {
            light.brightness += round(value);
        }
    } else {
        double perc = ((double)light.brightness / (double)light.max_brightness) * 100.0;
        if (dec && value >= perc) {
            light.brightness = 0;
        } else if (dec) {
            light.brightness -= round(((double)light.max_brightness / 100.0) * value);
        } else if (value + perc >= 100.0) {
            light.brightness = light.max_brightness;
        } else {
            light.brightness += round(((double)light.max_brightness / 100.0) * value);
        }
    }

    ulctl_error_t error = ulctl_light_write(&light);
    if (error.is_error) {
        fprintf(stderr, "Failed to write to device %s\n", light.name);
        ulctl_light_destroy(&light);
        udev_unref(udev);
        return 1;
    }

    if (!args->q) {
        ulctl_light_print(&light, args->m);
    }

    ulctl_light_destroy(&light);
    udev_unref(udev);

    return 0;
}
