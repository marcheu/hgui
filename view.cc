#include "includes.h"

#include "view.h"

static bool initialized = false;
static int width, height;

int view_width ()
{
	assert (initialized);
	return COLS - 1;
}

int view_height ()
{
	assert (initialized);
	return LINES - 1;
}

void view_init ()
{
	initscr ();

	keypad (stdscr, true);
	nodelay (stdscr, true);
	noecho ();
	ESCDELAY = 10;
	start_color ();
	curs_set (0);
	timeout (200);


	init_pair (1, COLOR_RED, COLOR_BLACK);
	init_pair (2, COLOR_GREEN, COLOR_BLACK);
	init_pair (3, COLOR_YELLOW, COLOR_BLACK);
	init_pair (4, COLOR_BLUE, COLOR_BLACK);
	init_pair (5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair (6, COLOR_CYAN, COLOR_BLACK);
	init_pair (7, COLOR_WHITE, COLOR_BLACK);
	init_pair (8, COLOR_BLACK, COLOR_GREEN);
	init_pair (9, COLOR_WHITE, COLOR_BLUE);

	width = COLS - 1;
	height = LINES - 1;

	initialized = true;
}

void view_close ()
{
	assert (initialized);
	echo ();
	curs_set (1);
	clear ();
	endwin ();
}

void view_refresh ()
{
	assert (initialized);
	refresh ();
}

int view_get_cursor_y ()
{
	int x, y;
	getyx (stdscr, x, y);
	(void) x;
	return y;
}
