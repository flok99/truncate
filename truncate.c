#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define __USE_LARGEFILE64
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "error.h"

off_t get_file_size(const char *const file_name)
{
	struct stat st;

	if (stat(file_name, &st) == -1)
		error_exit("Failed determing size of file %s: %s\n", file_name, strerror(errno));

	return st.st_size;
}

off_t get_file_size_fd(const int fd)
{
	struct stat st;

	if (fstat(fd, &st) == -1)
		error_exit("Failed determing size of file");

	return st.st_size;
}

void header(void)
{
	printf("truncate v" VERSION ", (C) by folkert@vanheusden.com\n\n");
}

void usage(void)
{
	header();
	printf("Please see the man-page for the switch -c, -r, -s and -v.\n");
}

int main(int argc, char *argv[])
{
	int argc_index = 1;
	char do_not_create = 0;
	off_t new_size = 0;
	char **truncate_files = NULL;
	int n_truncate_files = 0;
	char *do_what = NULL;
	char verbose = 0;
	char *as_file = NULL;
	int loop;

	while(argc_index < argc)
	{
		if (strcmp(argv[argc_index], "-c") == 0)
		{
			do_not_create = 1;
		}
		else if  (strcmp(argv[argc_index], "-r") == 0)
		{
			as_file = argv[++argc_index];
		}
		else if (strcmp(argv[argc_index], "-s") == 0)
		{
			do_what = argv[++argc_index];

			if (!(do_what[0] == '+' || do_what[0] == '-' || isdigit(do_what[0])))
			{
				usage();
				return 1;
			}
		}
		else if (strcmp(argv[argc_index], "-v") == 0)
		{
			verbose++;
		}
		else if (strcmp(argv[argc_index], "-h") == 0)
		{
			usage();
			return 0;
		}
		else if (strcmp(argv[argc_index], "-V") == 0)
		{
			header();
			return 0;
		}
		else
		{
			n_truncate_files++;
			truncate_files = (char **)realloc(truncate_files, sizeof(char *) * n_truncate_files);
			if (!truncate_files)
				error_exit("Memory allocation error");
			truncate_files[n_truncate_files - 1] = argv[argc_index];
		}

		argc_index++;
	}

	if ((!do_what && !as_file) || !truncate_files)
		error_exit("Do not know what to do. Either use -r or -s and specify files to process on.");

	if (verbose && do_not_create)
		printf("Will not create files if they do not exist.");

	if (as_file)
		new_size = get_file_size(as_file);

	for(loop=0; loop<n_truncate_files; loop++)
	{
		int fd;
		int flags = do_not_create ? 0 : O_CREAT;
		int mode = S_IRUSR | S_IWUSR | S_IRGRP;

		if (verbose)
			printf("Processing file %s\n", truncate_files[loop]);

		fd = open(truncate_files[loop], O_RDWR | flags, mode);
		if (fd == -1)
		{
			if (do_not_create && errno == ENOENT)
			{
				if (verbose)
					printf("File does not exist.\n");

				continue;
			}

			error_exit("Failed opening file %s: %s\n", truncate_files[loop], strerror(errno));
		}

		if (do_what)
		{
			char *str = do_what;
			char inc = 0;
			int len;

			if (do_what[0] == '+' || do_what[0] == '-')
			{
				inc = do_what[0];
				str = &do_what[1];
			}

			new_size = atoll(str);

			len = strlen(str);

			if (str[len - 1] == 'k' || str[len - 1] == 'K')
				new_size *= 1024;
			else if (str[len - 1] == 'm' || str[len - 1] == 'M')
				new_size *= 1024 * 1024;
			else if (str[len - 1] == 'g' || str[len - 1] == 'G')
				new_size *= 1024 * 1024 * 1024;
			else if (str[len - 1] == 't' || str[len - 1] == 'T')
				new_size *= ((unsigned long long int)1024 * 1024 * 1024) * 1024;

			if (inc)
			{
				off_t old_size = get_file_size_fd(fd);

				if (inc == '+')
					new_size += old_size;
				else if (inc == '-')
					new_size = old_size - new_size;
			}
		}

		if (verbose)
			printf("New size: %lld\n", (long long int)new_size);

		if (ftruncate(fd, new_size) == -1)
			error_exit("Failed truncating file %s to %lld bytes: %s\n", truncate_files[loop], new_size, strerror(errno));

		close(fd);
	}

	return 0;
}
