#include "drw.h"

static status_t drw_status_bar_fetch_text(Drw *, SAV *);
static void drw_text(Drw *drw, const char *text, int x, int y);

void drw_element_color(Drw *drw, int x, int y, int h, unsigned int col) {
	SDL_Rect rect;

	rect.x = x + drw->x_border; /* bottom left + x */
	rect.y = y - drw->y_border; /* bottom */
	rect.w = RECT_WIDTH; /* fixed width */
	rect.h = -h;

	SDL_SetRenderDrawColor(drw->rend, UNHEX(col));
	SDL_RenderFillRect(drw->rend, &rect);
	/* printf("INFO: color: #%02X%02X%02X%02X\n", UNHEX(col)); */

	/* Simulate shadows around rectangles */
	SDL_SetRenderDrawColor(drw->rend, UNHEX(0x000000FF));
	SDL_RenderDrawLine(drw->rend,
						x + drw->x_border + RECT_WIDTH - 1, y - drw->y_border - 1,
						x + drw->x_border + RECT_WIDTH - 1, y - drw->y_border - h);
}

void drw_array_graph(Drw *drw, SAV *sav) {
	int x;

	size_t i;
	for(i = x = 0; i < sav->arr->len; i++, x += RECT_WIDTH) {
		if(i == sav->sel) drw_element_color(drw, x, drw->h, sav->arr->v[i], SEL_COLOR);
		else if(i == sav->cmp) drw_element_color(drw, x, drw->h, sav->arr->v[i], CMP_COLOR);
		else drw_element_color(drw, x, drw->h, sav->arr->v[i], NORM_COLOR);
	}
}

static status_t drw_status_bar_fetch_text(Drw *drw, SAV *sav) {
	if(drw->bar_text == NULL) return ERROR_NULL_POINTER;

	if(sav->status == WELCOME) {
		snprintf(drw->bar_text, drw->bar_text_len - 2,
				"  Welcome to sorting algorithms visualized  [%s] [%-25s   press SPACE to start sorting",
				shuffle_t_str[sav->arr->shuffle_sel], algo_sel_str[sav->sort_algo]);
	}
	else if(sav->status == START) {
		snprintf(drw->bar_text, drw->bar_text_len - 2,
				"  %-8s  [%s] [%-25s   press SPACE to start sorting", sort_status_str[OK],
				shuffle_t_str[sav->arr->shuffle_sel], algo_sel_str[sav->sort_algo]);
	}
	else if(sav->status == RUN) {
		if(sav->sort_status == PAUSE)
			snprintf(drw->bar_text, drw->bar_text_len - 2,
					"  %-8s  [%s] [%-25s   L: %ld, C: %ld, S: %ld   Press SPACE to resume",
					sort_status_str[sav->sort_status],
					shuffle_t_str[sav->arr->shuffle_sel], algo_sel_str[sav->sort_algo],
					sav->arr->len, sav->cmps, sav->swps);
		else if(sav->sort_status == SORTED)
			snprintf(drw->bar_text, drw->bar_text_len - 2,
					"  %-8s  [%s] [%-25s   L: %ld, C: %ld, S: %ld, done in %lds, extra storage used: %ld Bytes",
					sort_status_str[sav->sort_status],
					shuffle_t_str[sav->arr->shuffle_sel], algo_sel_str[sav->sort_algo],
					sav->arr->len, sav->cmps, sav->swps, (sav->tf - sav->ti), sav->B_used);
		else if(sav->sort_status == RUN)
			snprintf(drw->bar_text, drw->bar_text_len - 2,
					"  %-8s  [%s] [%-25s   L: %ld, C: %ld, S: %ld", sort_status_str[sav->sort_status],
					shuffle_t_str[sav->arr->shuffle_sel], algo_sel_str[sav->sort_algo],
					sav->arr->len, sav->cmps, sav->swps);
	}
	else snprintf(drw->bar_text, drw->bar_text_len - 2, "  Exiting ..... ");

	return OK;
}

static void drw_text(Drw *drw, const char *text, int x, int y) {
	SDL_Surface *text_surface;
	SDL_Texture *text_texture;

	/* TODO: UNICODE support? */
	/* drw->text_surface = TTF_RenderText_Blended(drw->font, text, drw->text_color); */
	/* drw->text_surface = TTF_RenderUNICODE_Blended(drw->font, text, drw->text_color); */
	text_surface = TTF_RenderUTF8_Blended(drw->font, text, drw->text_color);
	text_texture = SDL_CreateTextureFromSurface(drw->rend, text_surface);

	drw->bar_text_rect.x = 10 + x;
	drw->bar_text_rect.y = drw->h - drw->font_size - 5;
	drw->bar_text_rect.w = text_surface->w;
	drw->bar_text_rect.h = text_surface->h;

	SDL_RenderCopy(drw->rend, text_texture, NULL, &drw->bar_text_rect);
	SDL_DestroyTexture(text_texture);
	SDL_FreeSurface(text_surface);
}

status_t drw_status_bar(Drw *drw, SAV *sav) {
	status_t st;
	SDL_Rect rect;
	int bar_border = 2;

	rect.x = bar_border; /* top left + x */
	rect.y = drw->h - bar_border; /* top left + y, (y < 0) */
	rect.w = drw->w - (2 * bar_border); /* fixed width */
	rect.h = -BAR_HEIGHT;

	/* TODO: Make a variable to store statusbar background color */
	SDL_SetRenderDrawColor(drw->rend, UNHEX(0x000000FF)); /* RGBA */
	SDL_RenderFillRect(drw->rend, &rect);

	if((st = drw_status_bar_fetch_text(drw, sav)) != OK) return st;

	drw_text(drw, drw->bar_text, 0, drw->h - drw->font_size - 5);
	memset(drw->bar_text, 0, sizeof(char) * drw->bar_text_len);

	return OK;
}

status_t Drw_create(Drw **drw) {
	SDL_Renderer *rend = NULL;
	SDL_Window *win = NULL;
	TTF_Font *font = NULL;
	status_t st;

	if(((*drw) = (Drw *)malloc(sizeof(Drw))) == NULL) return ERROR_MEMORY_ALLOC;
	memset(*drw, 0, sizeof(Drw));

	if((st = SDL_setup(&win, &rend)) != OK) return st;

	font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
	if(font == NULL) return ERROR_TTF_OPENING_FONT;

	(*drw)->rend = rend;
	(*drw)->win = win;
	(*drw)->font = font;
	(*drw)->font_size = FONT_SIZE;
	(*drw)->bar_border = 2;
	(*drw)->x_border = X_BORDER;
	(*drw)->y_border = Y_BORDER;

	(*drw)->text_color.r = (char)(FONT_COLOR >> 16) & 0xFF;
	(*drw)->text_color.g = (char)(FONT_COLOR >> 8) & 0xFF;
	(*drw)->text_color.b = (char)(FONT_COLOR) & 0xFF;

	SDL_GetWindowSize(win, &((*drw)->w), &((*drw)->h));

	(*drw)->bar_rect.x = (*drw)->bar_border; /* top left + x */
	(*drw)->bar_rect.y = (*drw)->h - (*drw)->bar_border; /* top left + y, (y < 0) */
	(*drw)->bar_rect.w = (*drw)->w - (2 * (*drw)->bar_border); /* fixed width */
	(*drw)->bar_rect.h = -BAR_HEIGHT;

	/* sometimes SDL_GetWindowSize() fails */
	if((*drw)->w < WIN_MIN_W)
		(*drw)->w = WIN_MIN_W;
	else if((*drw)->h < WIN_MIN_H)
		(*drw)->h = WIN_MIN_H;

	{
		int w_text, h_text;
		TTF_SizeText(font,
				"SORTED [XXXXXXXXXXXXXXXXXXXXX] done in XXX.Xs, L: XXXXX,\
				C: XXXXXX, S: XXXXXX, I: XXXXXX, storage used: XXXXXX Bytes",
				&w_text, &h_text);

		(*drw)->bar_text_len = w_text;
	}


	(*drw)->bar_text = (char *)malloc(sizeof(char) * (*drw)->bar_text_len);
	if((*drw)->bar_text == NULL) return ERROR_MEMORY_ALLOC;

	return OK;
}

void Drw_destroy(Drw *drw) {
	if(drw == NULL) return;

	TTF_CloseFont(drw->font);
	SDL_cleanup(drw->win, drw->rend);
	free(drw->bar_text);
	free(drw);
}
