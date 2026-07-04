# pico-raylib
A wrapper port of Raylib to boards with RP2350 and PSRAM

# Dependencies
- git cli - Must be placed on your PATH so that the cmake can init submodules recursively on the first use of this library.
- arm-none-eabi-gcc - Must be placed on your PATH for the pico sdk to find it.

# How to use
First, git clone this repository somewhere you will easily remember for your project.

In your project CMake, add_subdirectory() to the path you pulled this git repository to.  There's nothing else you need to do at all to start coding like any typical Raylib codebase; every dependency gets pulled from their respective git repositories automatically.  You just need to expect the disk usage to go up on first CMake configure.  Internet is required to update the submodules during that time.  Afterward it is no longer needed.

Set your CMake options as necessary.  This is mostly in regards to `RAYLIB_DISPLAY` and `RAYLIB_BOARD` as options.  It's recommended to do this via the CMake GUI.

All of your code should remain like you're using Raylib on any other machine.  You may add other pico libraries as you need but support for those hardware pins is not guaranteed as pico-raylib's backend might be claiming several of them.

A new header called `pico_display.h` should also be ready.  Include this so that you can control the RGBLED, screen orientation and display backlight.  You can check what's available by looking in the `optionals` folder then selecting the display you chose in the CMake options.

# Contributing
This is in regards to adding a new platform such as a pico board or a display output to this system.

## Adding a different Pico 2 Board.
Please note only RP2350 is supported by this repository, so only Pico 2 boards are possible.  Pico 1 support will not be accepted due to RAM and screen size constraints.

Pico board additions are fairly simple, usually only required to be committed as CMakeLists.txt changes.  Add your board as an enum option in the `RAYLIB_BOARD` option, then in the if-else tree set `PICO_BOARD` to the correct header from the Pico SDK.  

You will also want to do so again under pico_raylib's compiler definitions because you need to set the PSRAM pin to the one your board uses.  Otherwise the chip won't be selected and memory heavy apps will fail to allocate RAM.

## Adding a new display out.
This is much more complex.  Use `optionals/pimoroni_display_pack_28` as an example.

First, add it as an option to the CMake enum for `RAYLIB_DISPLAY`, making sure to be careful of the name you put in.

Next, copy and paste `templates/optionals_display` into `optionals` and rename that folder to the exact name you will used in CMake, except in lower case and with periods removed.  CMake is setup to automatically find this if your enum was selected for `RAYLIB_DISPLAY`.

Then, go through both the header and the c file to implement all of the drivers you need for your board type.

## Rules
Only C99 with the GNU99 extension code is allowed (I'm fully aware the Pico SDK uses some C++ and ends up building to CXX by force, but let's keep it standard.)
If need be this section may be extended in the future.

# Hardware Support TODO Checklist

- [ ] Pimoroni VGA Demo
- [ ] Pimoroni DVI Demo
- [x] Pimoroni Display Pack 2.8
- [x] Pimoroni Display Pack 1.14
- [x] Waveshare Pico LCD 1.3
