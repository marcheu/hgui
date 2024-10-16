#include "includes.h"

#include "blame.h"
#include "command_line.h"
#include "log.h"

#if 0
void show_commit ()
{
// popen("hg log -r commitid  -v -p -g")
}
#endif

typedef struct {
	const char *name;
	void (*func) (int argc, char *argv[]);
} function_table_entry;

function_table_entry function_table[] = {
	{"log", log_show},
	{"blame", blame_show},
	{"", NULL},
};


int parse_command_line (int argc, char *argv[])
{
	if (argc == 0)
		return -1;

	int i = 0;
	while (function_table[i].func != NULL) {
		if (!strcmp (function_table[i].name, argv[0])) {
			function_table[i].func (argc - 1, argv + 1);
			return 0;
		}
		i++;
	}

	return -1;
}
