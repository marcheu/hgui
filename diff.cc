#include "includes.h"

#include "constants.h"
#include "diff.h"
#include "util.h"
#include "view.h"

std::vector < char *>diff;
int diff_lines = 0;
int diff_cursor = 0;

void diff_read (char node[])
{
	static char current_diff[MAX_NODE_SIZE] = " ";

	if (!strcmp (current_diff, node))
		return;

	char cmd[MAX_CMD_SIZE];
	sprintf (cmd, "hg log -r %s  -v -p -g --stat --template compact", node);
	FILE *f = popen (cmd, "r");

	char buf[MAX_LINE_SIZE];
	// drop the first line
	readline (buf, MAX_LINE_SIZE, f);

	diff_cursor = 0;
	diff_lines = 0;
	diff.clear ();

	// Read the entire diff and put it in memory
	while (readline (buf, MAX_LINE_SIZE, f) != NULL) {
		diff.push_back (strdup (buf));
		diff_lines++;
	}
	pclose (f);

	strcpy (current_diff, node);
}

void diff_update_cursor ()
{
	if (diff_cursor < 0)
		diff_cursor = 0;
	if (diff_cursor >= diff_lines)
		diff_cursor = diff_lines - 1;
	if (diff_cursor + view_height () / 2 >= diff_lines)
		diff_cursor = max (0, diff_lines - view_height () / 2);
}
