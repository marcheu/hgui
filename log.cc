#include "includes.h"

#include "constants.h"
#include "database.h"
#include "diff.h"
#include "log.h"
#include "pager.h"
#include "util.h"
#include "view.h"

static pthread_t log_pthread;

static std::vector < char *>commit_log;

static void *log_thread (void *file)
{
	char *filename = (char *) file;

	// list all the commit revisions in the log
	char cmd[MAX_CMD_SIZE];
	sprintf (cmd, "hg log --template '{node}\n' ");
	if (filename)
		strcat (cmd, filename);

	FILE *f = popen (cmd, "r");
	if (!f)
		return NULL;

	char buf[MAX_NODE_SIZE];
	while (readline (buf, MAX_NODE_SIZE, f) != NULL) {
		// Force terminate the input
		buf[MAX_NODE_SIZE - 1] = 0;
		database_read_commit (buf);
		commit_log.push_back (strdup (buf));
	}

	pclose (f);

	return NULL;
}

static int log_update ()
{
	int line = 0;

	for (int i = start_line; i < min (start_line + list_size, (int) commit_log.size ()); i++, line++) {
		char node[MAX_NODE_SIZE];
		strcpy (node, commit_log[i]);

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

		// Date
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (4));
		attroff (A_BOLD);
		mvprintw (line, 0, "%.10s ", info->date);

		// Author
		attroff (A_BOLD);
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (2));
		printw (" %-18.18s ", info->author);

		// Commit hash
		attron (A_BOLD);
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else {
			attron (COLOR_PAIR (3));
		}
		printw ("%.8s ", info->node);

		// Revision number
/*		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (6));

		print_with_pad (commit_log[i], commit_log[0]);*/

		// Commit summary
		if (i == cursor)
			attron (COLOR_PAIR (8));
		else
			attron (COLOR_PAIR (7));
		printw ("| %s", info->summary);
	}

	move (list_size, 0);
	attron (COLOR_PAIR (9));
	attroff (A_BOLD);
	clrtoeol ();
	for (int x = 0; x < view_width (); x++)
		printw (" ");

	char node[MAX_NODE_SIZE];
	strcpy (node, commit_log[cursor]);
	const database_entry *info;
	info = database_get_commit (node);

	mvprintw (list_size, 0, "[log] %d/%lu   %s <%s>   %s   %s", cursor, commit_log.size () - 1, info->author, info->author_email, info->revision, info->node);

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

static int log_get_size ()
{
	return commit_log.size ();
}

static void log_get_commit (int cursor, char node[])
{
	strcpy (node, commit_log[cursor]);
}


int log_run (int argc, char *argv[])
{
	database_init ();

	char *file;
	if (argc > 0) {
		file = argv[0];
		if (!file_exists (file))
			return -1;
	}
	else {
		file = NULL;
	}

	pthread_create (&log_pthread, NULL, log_thread, file);

	pager_run (log_get_size, log_update, log_get_commit);

	pthread_join (log_pthread, NULL);

	database_close ();

	return 0;
}
