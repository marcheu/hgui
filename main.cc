#include "includes.h"

#include "command_line.h"
#include "constants.h"

// Check that dependencies are installed:
//  - hg
//  - iconv
//  - xml2
int check_dependencies ()
{
	if (system ("iconv /dev/null"))
		return -1;

	if (system ("xml2 --help 2> /dev/null") < 0)
		return -1;

	if (system ("hg --help > /dev/null"))
		return -1;

	return 0;
}

// Check that we are in a mercurial repository
int check_hg_repo ()
{
	char original_directory[MAX_FILE_SIZE];
	char current_directory[MAX_FILE_SIZE];
	if (!getcwd (original_directory, MAX_FILE_SIZE))
		return -ENOENT;

	while (true) {
		DIR *dir = opendir (".hg");

		if (dir) {
			// We found it! Go back to the original directory and proceed
			closedir (dir);
			if (chdir (original_directory) == -1)
				break;
			return 0;
		}

		// If we reached root directory and didn't find it, give up
		if (!strcmp (getcwd (current_directory, MAX_FILE_SIZE), "/"))
			break;

		// Try again with the next level up
		if (chdir ("..") == -1)
			break;
	}

	// Go back to the original directory and error out
	if (chdir (original_directory) == -1)
		return -ENOENT;
	return -ENOENT;
}



int main (int argc, char *argv[])
{
	int ret = 0;

	ret = check_dependencies ();
	if (ret) {
		printf ("Dependencies not found. Please install iconv xml2 and hg.\n");
		goto fail;
	}

	ret = check_hg_repo ();
	if (ret) {
		printf ("Not in a mercurial repo.\n");
		goto fail;
	}

	ret = parse_command_line (argc - 1, argv + 1);

      fail:
	return ret;
}
