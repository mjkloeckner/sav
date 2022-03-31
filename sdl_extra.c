#include "sdl_extra.h"

#define WINDOW_TITLE "SAV - Sorting Algorithms Visualized"

void
check_events(status_t *status) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT: *status = STOP; break;
		/* case SDL_KEYDOWN: */
		/* 	switch(event.key.keysym.scancode) { */
		/* 	case SDL_SCANCODE_EQUALS: */ 
		/* 		if(speed > SPEED_MAX) speed -= SPEED_STEP; */
		/* 		break; */
		/* 	case SDL_SCANCODE_MINUS: */
		/* 		if(speed < SPEED_MIN) speed += SPEED_STEP; */
		/* 		break; */
		/* 	case SDL_SCANCODE_P: */
		/* 		if(status == PAUSE) *status = RUN; */
		/* 		else *status = PAUSE; */
		/* 		break; */
		/* 	default: break; */
		/*    } */
		default: break;
		}
	}
}

void
setup(SDL_Window **win, SDL_Renderer **rend) {
	int min_w, min_h;

	SDL_Init(SDL_INIT_VIDEO);

    *win = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		0,
		0,
		SDL_WINDOW_OPENGL
    );

	*rend = SDL_CreateRenderer(*win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if(TTF_Init()==-1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	if (*win == NULL) end("SDL: window cannot be created");
	else if (*rend == NULL) end("SDL: renderer cannot be created");

	SDL_SetRenderDrawColor(*rend, 32, 32, 32, 0);
	SDL_RenderClear(*rend);
	SDL_RenderPresent(*rend);

	/* compute the window minimum size */
	min_w = ((ARR_LEN * RECT_WIDTH) + (2 * X_BORDER));
	min_h = ((ARR_MAX) + (2 * Y_BORDER) + TOP_BORDER);

	SDL_SetWindowMinimumSize(*win, min_w, min_h);
	SDL_SetWindowMaximumSize(*win, min_w, min_h);
}

void
cleanup(SDL_Window *win, SDL_Renderer *rend) {
	SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
	TTF_Quit();
	SDL_Quit();
}
