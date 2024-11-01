#include "includes.h"

#include "constants.h"
#include "database.h"
#include "util.h"

#define NUM_THREADS 16

static pthread_t database_pthread[NUM_THREADS];
static std::atomic_int active_threads;
static bool quit = false;

static std::map < uint64_t, database_entry > database;

static const database_entry not_found = {
	"              ",
	"              ",
	"              ",
	"              ",
	"              ",
	"              ",
	"              "
};

static std::queue < char *>job_queue;
static std::mutex job_mutex;

static std::mutex database_mutex;

static void add_revision_entry (database_entry * e, char *str)
{
	concat (&e->revision, str);
}

static void add_node_entry (database_entry * e, char *str)
{
	concat (&e->node, str);
}

static void add_author_email_entry (database_entry * e, char *str)
{
	concat (&e->author_email, str);
}

static void add_author_entry (database_entry * e, char *str)
{
	concat (&e->author, str);
}

static void add_date_entry (database_entry * e, char *str)
{
	concat (&e->date, str);
}

static void add_message_entry (database_entry * e, char *str)
{
	if (!e->summary)
		concat (&e->summary, str);

	concat (&e->message, str);
}

static void discard (database_entry * e, char *str)
{
}

static struct func_callback {
	char pattern[32];
	void (*callback) (database_entry * e, char *);
} callback_list[] = {
	{"/@revision=", add_revision_entry},
	{"/@node=", add_node_entry},
	{"/tag=", discard},
	{"/parent", discard},
	{"/author/@email=", add_author_email_entry},
	{"/author=", add_author_entry},
	{"/date=", add_date_entry},
	{"/msg/", discard},
	{"/msg=", add_message_entry},
	{"/branch=", discard},
	{"\0", discard},
	{"", NULL},
};

static void store_section (database_entry * e, char buf[])
{
	int i = 0;
	const int offset = strlen ("/log/logentry");

	if (strlen (buf + offset) == 1) {
		return;
	}

	while (callback_list[i].callback) {
		if (!strncmp (buf + offset, callback_list[i].pattern, strlen (callback_list[i].pattern))) {
			callback_list[i].callback (e, buf + offset + strlen (callback_list[i].pattern));
			return;
		}
		i++;
	}

	printf ("Did not find a match when parsing log entry %s.\n", buf);
	assert (0);
}


// requires database_mutex to be held
static bool have_commit (char *commit)
{
	return database.contains (node2int (commit));
}

void database_read_commit (char *node)
{
	// Add that read to our queue and return
	job_mutex.lock ();
	job_queue.push (strdup (node));
	job_mutex.unlock ();
}


static void database_do_read_commit (char *commit)
{
	database_mutex.lock ();
	if (have_commit (commit)) {
		database_mutex.unlock ();
		return;
	}
	database_mutex.unlock ();

	// The iconv part is a temporary hack because utf8 is hard
	char cmd[MAX_CMD_SIZE];
	sprintf (cmd, "hg log -Txml -r %s | xml2 | iconv -f utf-8 -t ascii//TRANSLIT ", commit);
	FILE *f = popen (cmd, "r");

	char buf[MAX_LINE_SIZE];

	database_entry e;
	bzero (&e, sizeof (database_entry));

	while (readline (buf, MAX_LINE_SIZE, f) != NULL) {
		store_section (&e, buf);
	}

	pclose (f);

	// We need to make sure writing the entire metadata is atomic
	database_mutex.lock ();
	database[node2int (e.node)] = e;
	database_mutex.unlock ();
}

void *database_thread (void *unused)
{
	while (quit == false) {
		usleep (100000);
		while (quit == false) {

			job_mutex.lock ();
			int size = job_queue.size ();
			if (size == 0) {
				job_mutex.unlock ();
				break;
			}

			active_threads++;

			char *entry = job_queue.front ();
			job_queue.pop ();
			job_mutex.unlock ();

			database_do_read_commit (entry);
			free (entry);
			active_threads--;
		}
	}

	return NULL;
}

void database_init ()
{
	database.clear ();

	active_threads = 0;
	for (int i = 0; i < NUM_THREADS; i++)
		pthread_create (&database_pthread[i], NULL, database_thread, NULL);
	assert (job_queue.size () == 0);
}

void database_close ()
{
	quit = true;

	for (int i = 0; i < NUM_THREADS; i++)
		pthread_join (database_pthread[i], NULL);

	for (auto it = database.begin (); it != database.end (); it++) {
		database_entry e = it->second;
		free (e.revision);
		free (e.message);
		free (e.summary);
		free (e.author);
		free (e.author_email);
		free (e.date);
		free (e.node);
	}
	database.clear ();
}

const database_entry *database_get_commit (char *node)
{
	if (database.contains (node2int (node)))
		return &database[node2int (node)];

	// We didn't find the entry, so let's return the special
	// "not found" entry

	return &not_found;
}

bool database_active ()
{
	job_mutex.lock ();
	bool ret = !job_queue.empty ();
	job_mutex.unlock ();

	// If any of our threads are active, then the database is
	// still loading
	if (active_threads > 0)
		return true;

	// Otherwise we are active if and only if the job queue is empty
	return ret;
}
