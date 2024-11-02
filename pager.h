extern int cursor;
extern int start_line;
extern int list_size;
extern bool show_commit;

void pager_run (int (*get_size) (), int (*update) (), void (*get_commit) (int, char[]));
