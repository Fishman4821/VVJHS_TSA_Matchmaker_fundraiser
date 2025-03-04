# How to Build

### Windows

First, download the portable gnu build tools [here](https://github.com/brechtsanders/winlibs_mingw/releases/download/14.2.0posix-19.1.7-12.0.0-msvcrt-r3/winlibs-x86_64-posix-seh-gcc-14.2.0-mingw-w64msvcrt-12.0.0-r3.zip).

Then, extract it and move the mingw64 folder into the directory with main.cpp

```
Matchmaker/
|   mingw64/
|   .gitignore
|   build.bat
|   main.cpp
|   README.md
```

Finally, inside the matchmaker folder, run `./build.bat`