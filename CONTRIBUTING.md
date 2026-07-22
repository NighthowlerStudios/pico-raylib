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

## NOTICE
All pull requests are subject to code review and may be changed to meet the formatting and standards set out in this repository, as seen fit.  Requests for changes may be frequent.