#include "includes.h"

#include "blame.h"
#include "constants.h"
#include "database.h"
#include "diff.h"
#include "pager.h"
#include "util.h"
#include "view.h"

static std::vector < char *>blame;

/* Hash a node name into a color and bold attribute */
static void hash_node (char *node)
{
	int res = node[0] + node[1] + node[2] + node[3] + node[4] + node[5] + node[6] + node[7];

	// use the top bits for color
	attron (COLOR_PAIR ((res >> 1) % 7 + 1));
	// use the bottom bit for bold attribute
	if (res % 2 == 0)
		attron (A_BOLD);
	else
		attroff (A_BOLD);
}

/* Extract the revision number from a blame line */
static void get_revision_for_line (int line, char revision[MAX_NODE_SIZE])
{
	if (line < 0) {
		revision[0] = '-';
		revision[1] = 0;
		return;
	}
	memcpy (revision, blame[line], 12);
	revision[12] = 0;
}

static int blame_update ()
{
	int line = 0;

	for (int i = start_line; i < min (start_line + list_size, (int) blame.size ()); i++, line++) {
		char *code;
		char node[MAX_NODE_SIZE];
		get_revision_for_line (i, node);
		code = blame[i];
		while (*code != ':')
			code++;
		code++;

		// Clear the entire line
		if (i == cursor) {
			attron (COLOR_PAIR (8));
		}
		else {
			attron (COLOR_PAIR (1));
		}
		move (line, 0);
		for (int x = 0; x < view_width (); x++)
			printw (" ");

		// Lookup the commit info from the database
		const database_entry *info;
		info = database_get_commit (node);

		// Commit hash
		attron (A_BOLD);
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else {
			attron (COLOR_PAIR (1));
			hash_node (info->node);
		}
		mvprintw (line, 0, "%.8s", info->node);

		// Author
		attroff (A_BOLD);
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (2));
		printw (" %-18.18s ", info->author);

		// Date
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (4));
		attroff (A_BOLD);
		mvprintw (line, 31, "%.10s ", info->date);

		// Line number
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (6));

		print_with_pad (i, blame.size ());

		// Code
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (7));
		printw ("| %s", code);
	}

	move (list_size, 0);
	attron (COLOR_PAIR (9));
	attroff (A_BOLD);
	clrtoeol ();
	for (int x = 0; x < view_width (); x++)
		printw (" ");

	char node[MAX_NODE_SIZE];
	get_revision_for_line (cursor, node);
	const database_entry *info;
	info = database_get_commit (node);

	mvprintw (list_size, 0, "[blame] %d/%lu   %s <%s>   %s   %s", cursor, blame.size () - 1, info->author, info->author_email, info->revision, info->node);

	if (show_commit) {
		move (list_size + 1, 0);
		attron (COLOR_PAIR (7));
		for (int i = diff_cursor; i < min (diff_cursor + view_height () / 2, (int) diff.size ()); i++) {
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
	}

	clear_rest_of_screen ();

	return 0;
}

static int blame_get_size ()
{
	return blame.size ();
}

static void blame_get_commit (int cursor, char node[])
{
	get_revision_for_line (cursor, node);
}

void blame_run (int argc, char *argv[])
{
	if (argc == 0)
		return;

	char cmd[MAX_CMD_SIZE];
	sprintf (cmd, "hg blame -c %s", argv[0]);
	FILE *f = popen (cmd, "r");
	if (!f)
		return;

	database_init ();

	char buf[MAX_LINE_SIZE];

	// Read the entire file and put it in memory
	// At the same time, request commit information for all the commits asynchronously
	while (readline (buf, MAX_LINE_SIZE, f) != NULL) {

		if (!strncmp (buf, "abort", 5))
			break;

		blame.push_back (strdup (buf));

		char node[MAX_NODE_SIZE];
		memcpy (node, buf, 12);
		buf[12] = 0;
		database_read_commit (node);
	}

	if (pclose (f) == -1) {
		database_close ();
		return;
	}

	pager_run (blame_get_size, blame_update, blame_get_commit);

	database_close ();
}
