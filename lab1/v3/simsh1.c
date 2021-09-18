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
		ssize_t command_length = read(fd, buf, PIPE_BUF * sizeof(char));
		if (command_length == -1)
			return -1;
		else if (command_length == 0) // EOF
			continue;

		// print prompt
		fprintf(stdout, "[%d]$ ", getpid());

		char *pid_str = strtok(buf, "\n");
		char *command = strtok(NULL, "\0");

		if (!pid_str || !command)
			return -1;

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
		close(fd);
		unlink(SERVER_FIFO_NAME);
		return -1;
	}

	int status = start_server();

	tear_down();

	return status;
}
