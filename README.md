# star_wallpaper
Animated wallpaper that looks like a moving field of stars, written as a C program

# Motivation
I got a new OLED panel in 2023 and wanted to have wallpaper that would minimize the potential for image burn-in and retention. Since my old wallpaper was a figure in a static field of stars, I figured I could program something that replicates that but with the stars moving and the figure floating around like a DVD screensaver (you know the ones). I found a wallpaper engine that allows arbitrary executables to be used as wallpapers and started coding just that. This is the results of those efforts so far. I chose C because I wanted to write something in C again and because I have previously explored SDL with C so I had the knowledge of how to do what I wanted using C.

# Compiling and Usage
I have compiled the program using the [Tiny C Compiler](http://www.tinycc.org/), but I see no reason why other compilers wouldn't work so long as the [SDL2](https://www.libsdl.org/) library is installed. The command I've been using is:
```
tcc ./starfield.c -lSDL2/SDL2
```

This will create an executable that can then be fed into your preffered animated wallpaper engine.
