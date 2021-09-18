// Simple shell example using fork() and execlp().

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include "../lib/parse_command.h"
#include "../lib/fifo_info.h"

int fd;

void tear_down()
{
	close(fd);
	unlink(SERVER_FIFO_NAME);
}

int start_server()
{
	pid_t k;
	char buf[PIPE_BUF];
	int status;

	while (1)
	{
		// read command from FIFO
		ssize_t command_len = read(fd, buf, PIPE_BUF);
		if (command_len == -1)
			return -1;
		else if (command_len == 0) // EOF
			continue;

		// split commands from buf with '\n' as delimiter
		char command[PIPE_BUF];
		size_t buf_idx = 0, command_idx = 0;
		while (buf_idx < command_len)
		{
			command_idx = 0;
			while (buf_idx < command_len && buf[buf_idx] != '\n')
				if (buf[buf_idx] == '\0') // ignore '\0' as we use '\n' instead
					buf_idx++;
				else
					command[command_idx++] = buf[buf_idx++];

			buf_idx++; // increase buf_idx to ignore '\n'

			if (command_idx == 0) // empty command
				continue;

			// close command string as we ignore it before
			command[command_idx] = '\0';

			// print prompt
			fprintf(stdout, "[%d]$ ", getpid());

			char *arguments[PIPE_BUF];
			parse_command(command, arguments);

			fflush(stdout); // flush stdout before forking
			k = fork();
			if (k == 0)
			{
				// child code
				int result = execvp(arguments[0], arguments);

				if (result == -1) // if execution failed, terminate child
				{
					fprintf(stderr, "Command not found: %s\n", command);
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				// parent code
				waitpid(k, &status, 0);
			}
		}
	}

	return 0;
}

void signal_handler(int _)
{
	tear_down();
	exit(EXIT_SUCCESS);
}

int main()
{
	signal(SIGINT, signal_handler);

	if (mkfifo(SERVER_FIFO_NAME, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH) == -1)
		return -1;
	fd = open(SERVER_FIFO_NAME, O_RDONLY);
	if (fd == -1)
	{
		unlink(SERVER_FIFO_NAME);
		return -1;
	}

	int status = start_server();

	tear_down();

	return status;
}
