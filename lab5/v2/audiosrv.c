#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../lib/audio_server.h"

static void sigint_handler(int _) { _exit(EXIT_SUCCESS); }

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    if (sigaction(SIGINT, &sigint_action, NULL) < 0)
    {
        perror("sigaction");
        return -1;
    }

    Config config;
    if (parse_args(argc, argv, &config) != 0) return -1;

    return start_server(&config);
}
