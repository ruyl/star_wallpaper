/* Due to some weird compatibility issue, main() is not recognized by (non-MinGW) TCC on Windows if handled by SDL, so we need to make SDL not handle main */
#ifdef _WIN32
#ifdef __TINYC__
#define SDL_MAIN_HANDLED 
#endif
#endif

#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <stdio.h>
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>
#include <windows.h>

/* Constants */
#define WINDOWX_POS SDL_WINDOWPOS_UNDEFINED
#define WINDOWY_POS SDL_WINDOWPOS_UNDEFINED
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800
#define WINDOW_FLAGS SDL_WINDOW_FULLSCREEN_DESKTOP 

#define SPAWN_MOD 40 /* number ticks in between spawning a new star */

#define MIN_SPEED 100
#define MAX_SPEED 100

/* ONLY USED IN MOTION TYPE 2- OTHERS USE THE NORMAL MIN AND MAX ONLY */
#define MIN_SPEED_Y 0
#define MAX_SPEED_Y 10

#define MAX_SIZE 10

#define EXTRA_EDGE_X MAX_SIZE
#define EXTRA_EDGE_Y MAX_SIZE

#define CENTER_VAR_X 100
#define CENTER_VAR_Y 100

#define MOTION_TYPE 2

struct window_struct {
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;
};

typedef struct star {
	struct star *next;
	float x_vel;
	float y_vel;
	float x_pos;
	float y_pos;
	int size;
} star;

struct window_struct* init_SDL() {
	/* Window to render to */
	SDL_Window* window = NULL;

	/* Surface contained by the window */
	SDL_Surface* screen_surface = NULL;

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL failed to initialize. SDL_Error: %s\n", SDL_GetError());
		return NULL;
	}

	/* Create window */
	window = SDL_CreateWindow
	(
		"star_wallpaper",						/* window's title*/
		WINDOWX_POS, WINDOWY_POS,				/* coordinates on the screen, in pixels, of the window's upper left corner*/
		SCREEN_WIDTH, SCREEN_HEIGHT,			/* window's length and height in pixels  */
		WINDOW_FLAGS
	);

	if (window == NULL) {
		printf("Failed to create window. SDL_Error: %s\n", SDL_GetError());
		return NULL;
	}

	/* Create window surface */
	screen_surface = SDL_GetWindowSurface(window);

	/* Create window renderer */
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

	/* Create the struct to return: */
	struct window_struct* return_window_struct = calloc(sizeof(struct window_struct), 1);
	return_window_struct->window = window;
	return_window_struct->surface = screen_surface;
	return_window_struct->renderer = renderer;

	return return_window_struct;
}

int parse_input() {
	SDL_Event e;
	
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			return 1;
		}
	}
	return 0;
}

void close_SDL(struct window_struct* window_data) {
	// SDL_FreeSurface(window_data->surface);	/* Deallocate surface OOPS not neccessary */
	SDL_DestroyWindow(window_data->window);	/* Destroy window */
	free(window_data);
	SDL_Quit();								/* Gracefully exit SDL */
}

int move_ball(star *all_stars) {
	star *starterator = all_stars->next;
	while (starterator != NULL) {
		starterator->x_pos += starterator->x_vel;
		starterator->y_pos += starterator->y_vel;
		starterator = starterator->next;
	}
}

/* Handles movement */
int handle_movement(star *all_stars) {
	move_ball(all_stars);
	return 0;
}

/* 
	Apparently this naive approach is a quick one. It does rely (I think) on the non-standardized convention that 0 is the only false bool value though 
	Used for randoms while generating star positions	
*/
double sign(double some_double) {
	return (some_double > 0) - (some_double < 0);
}

bool
check_within_window_size(SDL_Renderer *renderer, int x, int y)
{
	/* Explicitly initializing these as NULL just in case */
	int width = 0;
	int height = 0;
	SDL_GetRendererOutputSize(renderer, &width, &height);
	/* Remember that we must check to see if the width and height are null pointers */
	if (x < 0 - EXTRA_EDGE_X || y < 0 - EXTRA_EDGE_Y || width == 0 || height == 0 || x > width + EXTRA_EDGE_X || y > height + EXTRA_EDGE_Y) {
		return false;
	}
	return true;
}


void
purge_stars(SDL_Renderer *renderer, star *all_stars) {
	/* if game is not active, return immediately */

	/* it's NEXT here because the star list starts with a dummy value */
	star *starterator = all_stars->next;
	star *last = all_stars;
	while (starterator != NULL) {
		bool is_valid = true;
		is_valid = check_within_window_size(renderer, starterator->x_pos, starterator->y_pos);
		if (!is_valid) {
			last->next = starterator->next;
			free(starterator);
			starterator = last->next;
		} else {
			last = last->next;
			starterator = starterator->next;
		}
	}
}

/* Draws the 8 symmetrical points in a circle, given the radius of the circle, the unit point of the circle to draw, and it's x, y center point in the renderer coordinate system*/
void draw_8_symmetry(SDL_Renderer *renderer, int x, int y, int center_x, int center_y) {
		SDL_RenderDrawPoint(renderer, center_x + x, center_y - y);
		SDL_RenderDrawPoint(renderer, center_x + x, center_y + y);
		SDL_RenderDrawPoint(renderer, center_x - x, center_y - y);
		SDL_RenderDrawPoint(renderer, center_x - x, center_y + y);

		SDL_RenderDrawPoint(renderer, center_x + y, center_y - x);
		SDL_RenderDrawPoint(renderer, center_x + y, center_y + x);
		SDL_RenderDrawPoint(renderer, center_x - y, center_y - x);
		SDL_RenderDrawPoint(renderer, center_x - y, center_y + x);
}

/* Uses Bresenham's algorithm to draw the radius of a given circle, calculating midpoints without sqauring, using the previously calculated value (faster) */
void draw_circle_bresenham(SDL_Renderer* renderer, float center_x, float center_y, float radius)
{
	int x = 0;
	int y = radius;
	/* Precalulated distance between the midpoint of initial "east" and "southest" and the circle perimiter: */
	float dist = (float)5 / (float)4 - y;
	
	/* Draw the points on the approximate perimeter, but also every point below it */
	draw_8_symmetry(renderer, x, y, (int) center_x, (int) center_y);
	for (int downward_y = y; downward_y >= x; downward_y--) {
		draw_8_symmetry(renderer, x, downward_y, (int) center_x, (int) center_y);		
	}
	
	while (x < y) {
		/* If this point's distance is inside, then we choose "east" as a correction */
		if (dist <= 0) {
			x++;
			/* "Manually" calculate the next point's distance */
			dist = dist + 2 * x + 3;

		/* Else we choose "southeast" as a correction */
		} else {
			x++;
			y--;
			/* "Manually" calculate the next point's distance */
			dist = dist + 2 * x - 2 * y - 5;
		}
		/* Draw the points on the approximate perimeter, but also every point below it */
		draw_8_symmetry(renderer, x, y, (int) center_x, (int) center_y);
		for (int downward_y = y; downward_y >= x; downward_y--) {
			draw_8_symmetry(renderer, x, downward_y, (int) center_x, (int) center_y);		
		}
	}
}

int render(struct window_struct* window_data, star *all_stars) {
	/*
		REMEMBER: THE COORDINATE SYSTEM HAS ITS ORIGIN AT THE TOP LEFT
	*/
	SDL_Renderer* renderer = window_data->renderer;


	/* Fill black */
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer);

	int i;
	star *starterator = all_stars->next;
	while (starterator != NULL) {
		/* Fill the circle */
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		draw_circle_bresenham(renderer, starterator->x_pos, starterator->y_pos, starterator->size);
		starterator = starterator->next;
	}
	SDL_RenderPresent(renderer);
	return 0;
}


void
spawn_star(SDL_Renderer *renderer, star *all_stars)
{
	star *new_star = calloc(1, sizeof(star));
	int width = 0;
	int height = 0;
	
	new_star->size = rand() % MAX_SIZE;
	/* TODO: maybe put the "randomized value between given bounds" code into a function*/
	if (MOTION_TYPE == 1) {
		// SDL_GetWindowSize(window, width, height);
		SDL_GetRendererOutputSize(renderer, &width, &height);

		new_star->x_pos = width / 2 - CENTER_VAR_X + ((rand() % CENTER_VAR_X) % (2 * MAX_SPEED));
		new_star->y_pos = height / 2 - CENTER_VAR_Y + ((rand() % CENTER_VAR_Y) % (2 * MAX_SPEED));


		new_star->x_vel = ((rand() + MIN_SPEED) % MAX_SPEED) / (float) 100;
		new_star->y_vel = ((rand() + MIN_SPEED) % MAX_SPEED) / (float) 100;

		/* Generate random signs for x and y */
		int neg_y = (rand() - (RAND_MAX /2));
		int neg_x = (rand() - (RAND_MAX /2));
		new_star->x_vel *= sign(neg_x);
		new_star->y_vel *= sign(neg_y);
	}
	else if (MOTION_TYPE == 2) {
		// SDL_GetWindowSize(window, width, height);
		SDL_GetRendererOutputSize(renderer, &width, &height);

		new_star->x_pos = width + new_star->size;
		new_star->y_pos = ((rand() % height));


		new_star->x_vel = ((rand() + MIN_SPEED) % MAX_SPEED) / (float) 100;
		new_star->y_vel = ((rand() + MIN_SPEED_Y) % MAX_SPEED_Y) / (float) 100;
  
		/* Generate a random sign for y */
  		int neg_y = (rand() - (RAND_MAX /2));
		new_star->x_vel *= -1; /* for motion type 2 the stars should always be moving left */
		new_star->y_vel *= sign(neg_y); /* but we still want them to be able to skew up or down*/
	}

	new_star->next = all_stars->next;
	all_stars->next=new_star;
}

int
game_loop(struct window_struct* window_data){
	bool running = true;
	/* Main loop */

	/* STARS LIST STARTS WITH A DUMMY STAR! */
	star *all_stars = calloc(1, sizeof(star));
	all_stars->next = NULL;
	
	unsigned int tick_count = 0;
	while (running) {
		/* Event loop */
		/* handle inputs */
		if (parse_input() == 1) {
			/* if parse_input returns 1, SDL_QUIT has been sent, and the loop should stop */
			return 0;
		}
		if (tick_count % SPAWN_MOD == 0) {
			spawn_star(window_data->renderer, all_stars);
		}
		/* move stars based on velocity */
		handle_movement(all_stars);
		/* determine if stars are out of bounds and purge them */
		purge_stars(window_data->renderer, all_stars);
		/* draw to screen */
		render(window_data, all_stars);
		tick_count++;
	}
}

int main(int argc, char *argv[])
{
	struct window_struct* window_data = init_SDL();
	if (window_data == NULL) {
		return -1;
	}

	game_loop(window_data);
	close_SDL(window_data);					/* Gracefully exit SDL */
	return 0;
}


