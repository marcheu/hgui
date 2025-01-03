#include "constants.h"
#include "diff.h"
#include "util.h"
#include "view.h"

int cursor = 0;
int start_line = 0;
int list_size;
bool show_commit = false;


static void update_cursor (int (*get_size) (), void (*get_commit) (int, char[]))
{
	int current_size = get_size ();

	if (show_commit)
		list_size = view_height () / 2 - 1;
	else
		list_size = view_height () - 1;

	if (cursor < 0)
		cursor = 0;
	if (cursor < start_line)
		start_line = cursor;
	if (cursor >= current_size)
		cursor = current_size - 1;
	if (cursor >= start_line + list_size)
		start_line = cursor - list_size + 1;
	if (start_line + list_size >= current_size)
		start_line = current_size - list_size;

	diff_update_cursor ();

	if (show_commit) {
		char node[MAX_NODE_SIZE];
		get_commit (cursor, node);
		if (node[0] != '-')
			diff_read (node);
	}
}

void pager_run (int (*get_size) (), int (*update) (), void (*get_commit) (int, char[]))
{
	view_init ();
	list_size = view_height ();


	bool done = false;

	while (!done) {
		int c = getch ();
		flushinp ();

		switch (c) {
		case KEY_PPAGE:
			cursor -= view_height () / 2;
			break;

		case KEY_UP:
			cursor--;
			break;

		case KEY_NPAGE:
			cursor += view_height () / 2;
			break;

		case KEY_DOWN:
			cursor++;
			break;

		case KEY_IC:
			diff_cursor--;
			break;

		case KEY_DC:
			diff_cursor++;
			break;

		case 'q':
		case 27:
			if (show_commit) {
				show_commit = false;
			}
			else {
				done = true;
			}

			break;

		case '\n':
			show_commit = !show_commit;

			clear ();

			diff_cursor = 0;

			break;

		default:
			break;
		}

		update_cursor (get_size, get_commit);

		if (update ())
			done = true;

		view_refresh ();
	}

	view_close ();
}
