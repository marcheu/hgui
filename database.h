#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "constants.h"

typedef struct database_entry {
	char *revision;
	char *message;
	char *summary;
	char *author;
	char *author_email;
	char *date;
	char *node;
} database_entry;


void database_init ();
void database_close ();
void database_read_commit (char *node);
const database_entry *database_get_commit (char *node);
bool database_active ();

#endif
