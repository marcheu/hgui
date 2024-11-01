#ifndef _DIFF_H_
#define _DIFF_H_

#include "includes.h"

extern std::vector < char *>diff;
extern int diff_lines;
extern int diff_cursor;
void diff_read (char node[]);
void diff_update_cursor ();

#endif
