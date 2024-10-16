#include "includes.h"

#include "constants.h"
#include "diff.h"
#include "util.h"
#include "view.h"

std::vector < char *>diff;
int diff_lines = 0;
int diff_cursor = 0;

void diff_read (int commit_id)
{
	static int current_diff = -1;

	if (current_diff == commit_id)
		return;

	char cmd[MAX_CMD_SIZE];
	sprintf (cmd, "hg log -r %d  -v -p -g --stat --template compact", commit_id);
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

	current_diff = commit_id;
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
