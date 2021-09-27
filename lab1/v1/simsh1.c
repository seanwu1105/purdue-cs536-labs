// Simple shell example using fork() and execlp().

#include "../lib/parse_command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_LEN 4096

int main()
{
    pid_t k;
    char buf[BUFFER_LEN];
    int status;
    int len;

    while (1)
    {
        // print prompt
        fprintf(stdout, "[%d]$ ", getpid());

        // read command from stdin
        if (!fgets(buf, BUFFER_LEN, stdin)) break;

        len = strlen(buf);
        if (len == 1) // only return key pressed
            continue;

        char *arguments[BUFFER_LEN];
        parse_command(buf, arguments);

        fflush(stdout); // flush stdout before forking
        k = fork();
        if (k == 0)
        {
            // child code
            int result = execvp(arguments[0], arguments);

            if (result == -1) // if execution failed, terminate child
            {
                fprintf(stderr, "Command not found: %s\n", buf);
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
