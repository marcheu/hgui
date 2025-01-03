#include "includes.h"

#include "blame.h"
#include "command_line.h"
#include "log.h"
#include "show.h"

typedef struct {
	const char *name;
	int (*func) (int argc, char *argv[]);
} function_table_entry;

function_table_entry function_table[] = {
	{"log", log_run},
	{"blame", blame_run},
	{"show", show_run},
	{"", NULL},
};


int parse_command_line (int argc, char *argv[])
{
	if (argc == 0)
		return -1;

	int i = 0;
	while (function_table[i].func != NULL) {
		if (!strcmp (function_table[i].name, argv[0])) {
			return function_table[i].func (argc - 1, argv + 1);
		}
		i++;
	}

	return -1;
}
