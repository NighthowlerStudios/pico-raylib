# pico-raylib
A port of Raylib to boards with RP2350 and PSRAM

Please note that pimoroni-pico is a submodule included but not linked to anything, but is provided for ease of copy paste when reimplementing a CPP driver in C.  It may be removed from this repository in the future.

# How to use:
In your project CMake, add_subdirectory() to the path you pulled this git repository to.

Set your CMake options as necessary.  This is mostly in regards to `RAYLIB_DISPLAY` and `RAYLIB_BOARD` as options.

All of your code should remain like you're using Raylib.  You may add other pico libraries as you need but support for those hardware pins is not guaranteed.

A new header called `pico_display.h` should also be ready.  In here you can control the RGBLED and display backlight.  You can check what's available by looking in the `optionals` folder then selecting the display you chose in the CMake options.

# Contributing
This is in regards to adding a new platform such as a pico board or a display output to this system.

## Adding a different Pico 2 Board.
Please note only RP2350 is supported by this repository, so only Pico 2 boards are possible.

Pico board changes are fairly simple, usually only required to be committed as CMakeLists.txt changes.  Add your board as an enum option in the `RAYLIB_BOARD` option, then in the if-else tree set `PICO_BOARD` to the correct header from the Pico SDK.

## Adding a new display out.
This is much more complex.  Use `optionals/pimoroni_display_pack_28` as an example.

First, add it as an option to the CMake enum for `RAYLIB_DISPLAY`, making sure to be careful of the name you put in.

Next, copy and paste `templates/optionals_display` into `optionals` and rename that folder to the exact name you will used in CMake, except in lower case and with periods removed.  CMake is setup to automatically find this if your enum was selected for `RAYLIB_DISPLAY`.

Then, go through both the header and the c file to implement all of the drivers you need for your board type.

## Rules
Only C99 code is allowed (I'm fully aware the Pico SDK uses some C++ and ends up building to CXX by force, but let's keep it standard.)
If need be this section may be extended in the future.

# TODO Checklist

- [ ] Create a double buffer, then put the SPI communication onto Core 2, without copying the previous buffer (rlsw needs improvement)
- [ ] Get more examples to compile
- [ ] Implement Pimoroni VGA Demo
- [ ] Implement Pimoroni DVI Demo
- [ ] Implement SD card usage and file system via VGA Demo
- [ ] Implement audio over the Aux Jack on VGA Demo
- [ ] Accelerate matrix mathematics with the DSP
- [ ] Investigate and accelerate triangle span drawing using the rp2350 hardware interpolator