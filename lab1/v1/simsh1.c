// Simple shell example using fork() and execlp().

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "../lib/parse_command.h"

#define ARGUMENTS_LEN 100

int main()
{
	pid_t k;
	char buf[100];
	int status;
	int len;

	while (1)
	{

		// print prompt
		fprintf(stdout, "[%d]$ ", getpid());

		// read command from stdin
		fgets(buf, 100, stdin);
		len = strlen(buf);
		if (len == 1) // only return key pressed
			continue;
		buf[len - 1] = '\0';

		char *arguments[ARGUMENTS_LEN];
		parse_command(buf, arguments);

		fflush(stdout);
		k = fork();
		if (k == 0)
		{
			// child code
			int result = execvp(arguments[0], arguments);

			if (result == -1) // if execution failed, terminate child
			{
				printf("Command not found: %s\n", buf);
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
