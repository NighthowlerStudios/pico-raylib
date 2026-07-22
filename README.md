# pico-raylib
A wrapper port of Raylib to boards with RP2350 and PSRAM

# Dependencies
- git cli - Must be placed on your PATH so that the cmake can init submodules recursively on the first use of this library.
- arm-none-eabi-gcc - Must be placed on your PATH for the pico sdk to find it.

# How to use
First, git clone this repository somewhere you will easily remember for your project.

In your project CMake, add_subdirectory() to the path you pulled this git repository to.  At the end of it make sure to use
```
    if(SD_CARD)
        pico_enable_filesystem(fs_init_example FS_INIT "pico-vfs/examples/fs_inits/fs_init_fat_sdcard.c")
    endif()
    pico_add_extra_outputs(example)
```

Every dependency gets pulled from their respective git repositories automatically.  You just need to expect the disk usage to go up on first CMake configure.  Internet is required to update the submodules during that time.  Afterward it is no longer needed.

Set your CMake options as necessary.  This is mostly in regards to `RAYLIB_DISPLAY` and `RAYLIB_BOARD` as options.  It's recommended to do this via the CMake GUI.

All of your code should remain like you're using Raylib on any other machine.  You may add other pico libraries as you need but support for those hardware pins is not guaranteed as pico-raylib's backend might be claiming several of them.

A new header called `pico_display.h` should also be ready.  Include this so that you can control the RGBLED, screen orientation and display backlight (if these are available).  You can check what's available by looking in the `optionals` folder then selecting the display you chose in the CMake options.

# Hardware Support TODO Checklist

- [ ] Pimoroni VGA Demo
- [ ] Pimoroni DVI Demo
- [x] Pimoroni Display Pack 2.8
- [x] Pimoroni Display Pack 1.14
- [ ] Waveshare Pico LCD 1.3
- [ ] Adafruit 1.9 320x170 Color IPS TFT Display
- [ ] Adafruit SHARP Memory Display Breakout
