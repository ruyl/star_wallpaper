all: star_wallpaper

star_wallpaper:
	tcc ./star_wallpaper.c -lSDL2/SDL2

clean:
	del star_wallpaper.exe