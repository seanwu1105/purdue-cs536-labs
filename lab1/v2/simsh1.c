// Simple shell example using fork() and execlp().

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "parse_command.h"
#include "fifo_info.h"

#define ARGUMENTS_SIZE 100

int read_command(char *fifo_filename, char *command)
{
	int fd = open(fifo_filename, O_RDONLY);
	if (fd == -1)
	{
		close(fd);
		return -1;
	}
	if (read(fd, command, sizeof(command)) == -1)
	{
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int start_server(char *fifo_filename)
{
	pid_t k;
	char buf[100];
	int status;

	while (1)
	{

		// print prompt
		fprintf(stdout, "[%d]$ ", getpid());

		// read command from FIFO
		if (read_command(fifo_filename, buf))
			return -1;

		char *arguments[ARGUMENTS_SIZE];
		parse_command(buf, arguments);

		k = fork();
		if (k == 0)
		{
			// child code
			int status = execvp(arguments[0], arguments);
			clear_arguments(arguments);
			if (status == -1) // if execution failed, terminate child
				exit(1);

			printf("\n");
		}
		else
		{
			// parent code
			waitpid(k, &status, 0);
		}
	}

	return 0;
}

int main()
{
	if (mkfifo(FIFO_FILENAME, S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH) == -1)
		return -1;

	printf("FIFO created.\n");

	int status = start_server(FIFO_FILENAME);
	unlink(FIFO_FILENAME);
	if (status == -1)
		return -1;
	return 0;
}