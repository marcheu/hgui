#include "includes.h"

#include "constants.h"
#include "database.h"
#include "util.h"

#define NUM_THREADS 16

static pthread_t database_pthread[NUM_THREADS];
static std::atomic_int active_threads;
static bool quit = false;

static int database_index = -1;
static std::vector < database_entry > database;

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

static void add_revision_entry (char *str)
{
	database_index++;
	database.resize (database_index + 1);
	database_entry *de = &database[database_index];
	bzero (de, sizeof (database_entry));

	concat (&de->revision, str);
}

static void add_node_entry (char *str)
{
	database_entry *de = &database[database_index];
	concat (&de->node, str);
}

static void add_author_email_entry (char *str)
{
	database_entry *de = &database[database_index];
	concat (&de->author_email, str);
}

static void add_author_entry (char *str)
{
	database_entry *de = &database[database_index];
	concat (&de->author, str);
}

static void add_date_entry (char *str)
{
	database_entry *de = &database[database_index];
	concat (&de->date, str);
}

static void add_message_entry (char *str)
{
	database_entry *de = &database[database_index];

	if (!de->summary)
		concat (&de->summary, str);

	concat (&de->message, str);
}

static void discard (char *str)
{
}

static struct func_callback {
	char pattern[32];
	void (*callback) (char *);
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
	{"\0", NULL},
};

// requires database_mutex to be held
static void store_section (char buf[])
{
	int i = 0;
	const int offset = strlen ("/log/logentry");

	if (strlen (buf + offset) == 1) {
		return;
	}

	while (callback_list[i].callback) {
		if (!strncmp (buf + offset, callback_list[i].pattern, strlen (callback_list[i].pattern))) {
			callback_list[i].callback (buf + offset + strlen (callback_list[i].pattern));
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
	int i = 0;
	while (i < database.size ()) {
		if (!strcmp (database[i].revision, commit)) {
			return true;
		}
		i++;
	}
	return false;
}

void database_read_commit (int commit_id)
{
	char commit_str[128];
	sprintf (commit_str, "%d", commit_id);

	// Add that read to our queue and return
	job_mutex.lock ();
	job_queue.push (strdup (commit_str));
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
	wait_file_readable (f);

	// We need to make sure writing the entire metadata is atomic
	database_mutex.lock ();
	while (readline (buf, MAX_LINE_SIZE, f) != NULL) {
		store_section (buf);
	}
	database_mutex.unlock ();

	pclose (f);
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

	for (int i = 0; i <= database_index; i++) {
		free (database[i].revision);
		free (database[i].message);
		free (database[i].summary);
		free (database[i].author);
		free (database[i].author_email);
		free (database[i].date);
		free (database[i].node);
	}
}

const database_entry *database_get_commit (int commit_id)
{
	char commit_str[128];
	sprintf (commit_str, "%d", commit_id);

	int i = 0;
	while (i < database.size ()) {
		if (!strcmp (database[i].revision, commit_str)) {
			return &database[i];
		}
		i++;
	}

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
