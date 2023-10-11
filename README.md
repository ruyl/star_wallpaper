# star_wallpaper
Animated wallpaper that looks like a moving field of stars, written as a C program

# Motivation
I got a new OLED display in 2023 and wanted to have wallpaper that would minimize the potential for image burn-in and retention. Since my old wallpaper was a figure in a static field of stars, I figured I could program something that replicates that but with the stars moving and the figure floating around like a DVD screensaver (you know the ones). I found a wallpaper engine that allows arbitrary executables to be used as wallpapers and started coding just that. This is the results of those efforts so far. I chose C because I wanted to write something in C again and because I have previously explored SDL with C so I had the knowledge of how to do what I wanted using C.

# Compiling and Usage
Note: This program has only been tested on Windows, but should work on other systems that have SDL2.

I have compiled the program using the [Tiny C Compiler](http://www.tinycc.org/), but I see no reason why other compilers wouldn't work so long as the [SDL2](https://www.libsdl.org/) library is installed. The command I've been using is:
```
tcc ./star_wallpaper.c -lSDL2/SDL2
```
But you may also need to include SDL2Main.  
I have also included a makefile in the repository for building on windows. Simply run ```make``` and it should work.

This will create an executable that can then be fed into your preffered animated wallpaper engine.

# Features and Future Enhancements
Currently the wallpaper has two modes: stars flying from the right to the left of the screen and stars coming out from the middle of the screen (like the old Windows screensaver). The former is default.  
All configuration of the screensaver including details like speed and amount of stars can be modulated by changing the values at the top of the file before compiling.  
As of now, choosing an image (a "mascot" so to speak) that will float around as was originally intended has not yet been implemented, but will be added once I look at my wallpaper and think to myself that something else is needed.  
For a few days after I first coded this I thought for a while about how this could be improved and so there are some things that I still remember from that time:  
1. For the stars coming out of the center of the screen motion (MOTION_TYPE 1), the stars should spawn from a set distance away from the center instead of from the center. This looks more realistic.
2. For the stars coming from the right side of the screen motion (MOTION_TYPE 2), it would be more efficient (and make memory usage more deterministic) to, rather than continuing to spawn a star every SPAWN_MOD ticks and then destroy them when they go out of bounds, spawn a set number of stars at the begining of the program's execution, and simply reset their positions and velocities to initial randomized states at the right side of the screen when they go out of bounds. This number would be based on the size of the screen in order to make sure the star field doesn't feel too empty. This would not only allow the program to use a set amount of memory, but is more efficient as well since it eliminates the need for linked lists (the stars could be stored in a simple array of structs instead).
3. The rendering code for the stars is unoptimized, looks weird (because it just draws progressively smaller circle perimeters instead of checking to draw points inside the circle, leaving gaps) and should possibly be replaced by set sprites per size.
4. Motion 2 might look better if, instead of having any random speed between two points, the stars moved in speed "cohorts", closer replicating a paralax effect for stars that have clustered together.
