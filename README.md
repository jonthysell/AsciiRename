# AsciiRename #

[![CI Build](https://github.com/jonthysell/AsciiRename/actions/workflows/ci.yml/badge.svg)](https://github.com/jonthysell/AsciiRename/actions/workflows/ci.yml)

AsciiRename is a small command-line tool for renaming files with Unicode filenames into transliterated ASCII equivalents.

It was made primarily to assist with managing media files destined for devices with no/limited support for Unicode filenames.

AsciiRename relies on [AnyAscii](https://github.com/anyascii/anyascii) to perform the transliteration and [libpu8](https://github.com/jofeu/libpu8) to build cross-platform on both Windows and Linux systems.

## Installation ##

### Windows ###

The Windows release provides self-contained x86/x64 binaries which run on Windows.

1. Download the latest Windows zip file from https://github.com/jonthysell/AsciiRename/releases/latest
2. Extract the zip file

**Note:** If you're unsure which version to download, try x64. Most modern PCs are 64-bit.

### MacOS ###

The MacOS release provides self-contained x64 binaries which run on OSX.

1. Download the latest MacOS tar.gz file from https://github.com/jonthysell/AsciiRename/releases/latest
2. Extract the tar.gz file

### Linux ###

The Linux release provides self-contained x64 binaries which run on many Linux distributions.

1. Download the latest Linux tar.gz file from https://github.com/jonthysell/AsciiRename/releases/latest
2. Extract the tar.gz file

## Usage ##

```none
Usage: ascii-rename [options...] [paths...]
-h, --help       Show this help and exit
-n, --no-op      Show what would happen but don't actually rename path(s)
-o, --overwrite  Overwrite existing paths(s)
-r, --recursive  Rename files and subdirectories recursively
-v, --verbose    Make the output more verbose
-V, --version    Show version number and exit
```

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

Copyright (c) 2022-2024 Jon Thysell

AnyAscii Copyright (c) 2020-2023, Hunter WB <hunterwb.com>

libpu8 Copyright (c) 2019, jofeu
