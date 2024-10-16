#ifndef _DATABASE_H_
#define _DATABASE_H_

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
void database_read_commit (int commit_id);
const database_entry *database_get_commit (int commit_id);
bool database_active ();

#endif
