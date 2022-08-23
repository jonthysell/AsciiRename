# AsciiRename #

AsciiRename is a small command-line tool for renaming files with Unicode filenames into transliterated ASCII equivalents.

It was made primarily to assist with managing media files destined for devices with no/limited support for Unicode filenames. 

## Build ##

This project requires CMake >= 3.16 and a standard C++ build environment.

### Windows (VS) ###

Open VS Command Prompt, then run one of the following:

* Build Debug: `.\scripts\build-debug.cmd`
* Build Release: `.\scripts\build-release.cmd`

### Linux (GCC) / MacOS (AppleClang) ###

* Build Debug: `./scripts/build-debug.sh`
* Build Release: `./scripts/build-release.sh`

### General ###

```
mkdir build
cd build
cmake ..
cmake --build .
```

## Errata ##

AsciiRename is open-source under the MIT license.

Copyright (c) 2022 Jon Thysell.

AnyAscii Copyright (c) 2020-2022 Hunter WB
