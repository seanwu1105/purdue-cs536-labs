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

int server_fifo_fd;

void tear_down()
{
	close(server_fifo_fd);
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
		ssize_t command_len = read(server_fifo_fd, buf, PIPE_BUF);
		if (command_len == -1)
			return -1;
		else if (command_len == 0) // EOF
			continue;			   // busy wait for the new command from clients

		char *pid_str = strtok(buf, "\n");
		char *command = strtok(NULL, "\0");

		if (!pid_str || !command)
			return -1;

		fprintf(stdout, "[%s]$ %s", pid_str, command);

		char *arguments[PIPE_BUF];
		parse_command(command, arguments);

		int client_fifo_fd;

		fflush(stdout); // flush stdout before forking
		k = fork();
		if (k == 0)
		{
			// child code

			// open client FIFO according to the parsed pid
			char client_fifo_filename[100] = CLIENT_FIFO_NAME_PREFIX;
			strcat(client_fifo_filename, pid_str);
			client_fifo_fd = open(client_fifo_filename, O_WRONLY);
			if (client_fifo_fd == -1)
				return -1;
			// redirect stdout to client FIFO file descriptor
			if (dup2(client_fifo_fd, STDOUT_FILENO) == -1 || dup2(client_fifo_fd, STDERR_FILENO) == -1)
				exit(EXIT_FAILURE);

			int result = execvp(arguments[0], arguments);

			if (result == -1) // if execution failed, terminate child
			{
				fprintf(stderr, "Command not found: %s", command);
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			// parent code
			waitpid(k, &status, 0);
			close(client_fifo_fd);
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
	server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
	if (server_fifo_fd == -1)
	{
		unlink(SERVER_FIFO_NAME);
		return -1;
	}

	int status = start_server();

	tear_down();

	return status;
}
