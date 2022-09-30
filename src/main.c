#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool initialize_window(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error: SDL_Init() failed\n");
		return false;
	}

    return true;
}

int main(void) {

	initialize_window();

	return 0;
}
