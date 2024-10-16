#ifndef _UTIL_H_
#define _UTIL_H_

#include "includes.h"

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

void wait_file_readable (FILE * f);
char *readline (char *buf, int size, FILE * f);
uint64_t get_time ();
void print_with_pad (int num, int max_num);
void clear_rest_of_screen ();
void concat (char **s1, char *s2);

#endif
