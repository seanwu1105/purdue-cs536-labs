// Simple shell example using fork() and execlp().

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../lib/parse_command.h"
#include "fifo_info.h"

#define BUFFER_SIZE 100

int read_command(char *fifo_filename, char *command, size_t command_size)
{
	int fd = open(fifo_filename, O_RDONLY);
	if (fd == -1)
	{
		close(fd);
		return -1;
	}
	int command_length = read(fd, command, sizeof(char) * command_size);
	close(fd);
	return command_length;
}

int start_server(char *fifo_filename)
{
	pid_t k;
	char buf[BUFFER_SIZE];
	int status;

	while (1)
	{
		// read command from FIFO
		int command_length = read_command(fifo_filename, buf, BUFFER_SIZE);
		if (command_length == -1)
			return -1;
		else if (command_length == 0) // EOF
			continue;

		// print prompt
		fprintf(stdout, "[%d]$ ", getpid());

		char *arguments[BUFFER_SIZE];
		parse_command(buf, arguments);

		fflush(stdout);
		k = fork();
		if (k == 0)
		{
			// child code
			int result = execvp(arguments[0], arguments);
			clear_arguments(arguments);

			if (result == -1) // if execution failed, terminate child
				exit(EXIT_FAILURE);
		}
		else
		{
			// parent code
			waitpid(k, &status, 0);
		}
	}

	return 0;
}

void signal_handler(int _)
{
	unlink(FIFO_FILENAME);
	exit(EXIT_SUCCESS);
}

int main()
{
	if (mkfifo(FIFO_FILENAME, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH) == -1)
		return -1;

	signal(SIGINT, signal_handler);

	int status = start_server(FIFO_FILENAME);

	unlink(FIFO_FILENAME);
	if (status == -1)
		return -1;
	return 0;
}

// should test: multiple commands in single pipe write
// should test: atomic write
// should test: multiple client long tasks