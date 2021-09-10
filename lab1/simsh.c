// Simple shell example using fork() and execlp().

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "parse_command.h"

int main(void)
{
	// char buf[100] = "ls -a --version -c hello -s \"hello world\" --escape \"Escape before \\\" after\"";
	// char *arguments[100];
	// int r = parse_command(buf, arguments);

	// size_t i = 0;

	// while (arguments[i])
	// {
	// 	printf("%s[END]\n", arguments[i++]);
	// }

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

		// char *arguments[100];
		// int r = parse_command(buf, arguments);

		// size_t i = 0;

		// while (arguments[i])
		// {
		// 	printf("%s[END]\n", arguments[i++]);
		// }

		k = fork();
		if (k == 0)
		{
			// child code
			if (execlp(buf, buf, NULL) == -1) // if execution failed, terminate child
				exit(1);
		}
		else
		{
			// parent code
			waitpid(k, &status, 0);
		}
	}
}
