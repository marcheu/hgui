#include "constants.h"
#include "util.h"
#include "view.h"

/* Blocks until a file is readable */
void wait_file_readable (FILE * f)
{
	int fd = fileno (f);
	fd_set read_set;
	FD_ZERO (&read_set);
	FD_SET (fd, &read_set);

	while (select (fd + 1, &read_set, NULL, NULL, NULL) < 0) {
	}

}

/* reads a line from a file but strips \n and \r */
char *readline (char *buf, int size, FILE * f)
{
	char *ret = fgets (buf, MAX_LINE_SIZE, f);
	if (ret) {
		// remove \n and \r
		ret[strcspn (ret, "\r\n")] = 0;
	}
	return ret;
}

/* Get a 64 bit microsecond time */
uint64_t get_time ()
{
	struct timeval t;
	gettimeofday (&t, NULL);
	return (uint64_t) t.tv_sec * 1000000ULL + (uint64_t) t.tv_usec;
}

/* How many characters are needed to represent that int? */
static int num_characters (int n)
{
	if (n == 0)
		return 1;
	return floor (log10 (n)) + 1;
}

/* Print an integer with the padding for the number of characters */
void print_with_pad (int num, int max_num)
{
	static int line_num_char_count = num_characters (max_num);
	int num_char_count = num_characters (num);

	for (int i = 0; i < line_num_char_count - num_char_count; i++)
		printw (" ");
	printw ("%d", num);
}

/* Clears the rest of the screen starting at the cursor */
void clear_rest_of_screen ()
{
	for (int i = view_get_cursor_y (); i < view_height (); i++) {
		attron (COLOR_PAIR (1));
		attroff (A_BOLD);
		clrtoeol ();
		for (int x = 0; x < view_width (); x++)
			printw (" ");
	}

}

/* Concatenates s1 and s2, with the result in s1
 * Reallocates s1 if needed
 */
void concat (char **s1, char *s2)
{
	if (!*s1 && !s2)
		return;

	if (!s2)
		return;

	if (!*s1) {
		*s1 = strdup (s2);
		return;
	}

	char *ret = (char *) malloc (strlen (*s1) + strlen (s2) + 1);
	sprintf (ret, "%s%s", *s1, s2);
	free (*s1);
	*s1 = ret;
}
