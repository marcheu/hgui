#include "includes.h"

#include "constants.h"
#include "database.h"
#include "diff.h"
#include "pager.h"
#include "show.h"
#include "util.h"
#include "view.h"

static char *commit_id;

static int show_update ()
{
	if (!show_commit)
		return 1;

	move (0, 0);
	attron (COLOR_PAIR (7));
	for (int i = diff_cursor; i < min (diff_cursor + view_height (), (int) diff.size ()); i++) {
		switch (diff[i][0]) {
		case '-':
			attron (A_BOLD);
			attron (COLOR_PAIR (1));
			break;
		case '+':
			attron (A_BOLD);
			attron (COLOR_PAIR (2));
			break;
		case '@':
			attron (A_BOLD);
			attron (COLOR_PAIR (5));
			break;
		default:
			attroff (A_BOLD);
			attron (COLOR_PAIR (7));
			break;
		}
		if (i == 0)
			attron (A_BOLD);
		printw ("%s\n", diff[i]);
	}

	return 0;
}

static int show_get_size ()
{
	return 0;
}

static void show_get_commit (int cursor, char node[])
{
	strcpy (node, commit_id);
}


int show_run (int argc, char *argv[])
{
	if (argc == 0)
		return -1;

	commit_id = strdup (argv[0]);

	show_commit = true;

	pager_run (show_get_size, show_update, show_get_commit);

	return 0;
}
