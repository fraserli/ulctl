<div align="center">

# `ulctl`

`ulctl` is CLI application to control light devices in Linux.

</div>

## Example Usage
```shell
# Show backlight information
$ ulctl
Device "intel_backlight" (backlight):
    Current brightness: 14400 (15.00%)
    Max brightness: 96000

# Set brightness to 20%
$ ulctl set 20
Device "intel_backlight" (backlight):
    Current brightness: 19200 (20.00%)
    Max brightness: 96000

# Increase brightness by 2.5%
$ ulctl inc 2.5
Device "intel_backlight" (backlight):
    Current brightness: 21600 (22.50%)
    Max brightness: 96000

# Change brightness of non-default device
$ ulctl -d dell::kbd_backlight set 100
Device "dell::kbd_backlight" (leds):
    Current brightness: 2 (100.00%)
    Max brightness: 2

# For more options
$ ulctl -h
```
