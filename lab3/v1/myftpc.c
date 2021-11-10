#include "arg_checkers.h"
#include "request_codec.h"
#include "socket_utils.h"
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define REQUIRED_ARGC 6
#define TIMEOUT_SEC 2

int sockfd = -1;

void tear_down() { close(sockfd); }

static void sigint_handler(int _)
{
    tear_down();
    _exit(EXIT_SUCCESS);
}

typedef struct
{
    char *filename;
    uint16_t secret_key;
    unsigned long blocksize_byte;
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port> <filename> <secret-key> "
                "<blocksize-byte>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_STREAM)) !=
        0)
        return status;

    config->filename = argv[3];
    if ((status = check_filename(config->filename)) != 0) return status;

    long long secret_key = strtoull(argv[4], NULL, 0);
    if ((status = check_secret_key(secret_key)) != 0) return status;
    config->secret_key = (uint16_t)secret_key;

    config->blocksize_byte = strtoul(argv[5], NULL, 0);

    return 0;
}

int append_file(const char *const filename, const uint8_t *const data,
                const size_t data_size)
{
    FILE *file = fopen(filename, "a");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    fwrite(data, sizeof(uint8_t), data_size, file);

    fclose(file);

    if (ferror(file))
    {
        perror("fwrite");
        return -1;
    }

    return 0;
}

ssize_t fetch_and_save_file(const Config *const config)
{
    // Check if file exists
    if (access(config->filename, F_OK) == 0)
    {
        fprintf(stdout, "File already exists: %s\n", config->filename);
        return -1;
    }

    // Read response from server
    size_t total_bytes_read = 0;
    uint8_t buffer[config->blocksize_byte];
    ssize_t bytes_read;
    while (bytes_read = read(sockfd, buffer, sizeof(buffer)), bytes_read > 0)
    {
        // Write into disk
        append_file(config->filename, buffer, bytes_read);
        total_bytes_read += bytes_read;
    }

    if (bytes_read < 0)
    {
        perror("read");
        return -1;
    }

    return total_bytes_read;
}

void print_statistics(ssize_t total_bytes_read, struct timeval start_time)
{
    fprintf(stdout, "Total fetched: %ld bytes\n", total_bytes_read);

    if (total_bytes_read == 0)
        fprintf(
            stderr,
            "No data received. See message from server to get the reason\n");

    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    double completion_time_millis =
        (end_time.tv_sec - start_time.tv_sec) * 1000 +
        (end_time.tv_usec - start_time.tv_usec) / 1000.0;
    fprintf(stdout, "Completion time: %.3lf ms\n", completion_time_millis);

    fprintf(stdout, "Throughput: %.3lf bytes/ms\n",
            total_bytes_read / completion_time_millis);
}

int run(const struct addrinfo *const server_info, const Config *const config)
{
    // Create socket
    if ((sockfd = create_socket_with_first_usable_addr(server_info)) == -1)
        return -1;

    // Connect to server
    if (connect(sockfd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("connect");
        return -1;
    }

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Request file
    uint8_t request[REQUEST_SIZE];
    encode_request(config->filename, config->secret_key, request);
    if (write(sockfd, request, sizeof(request)) == -1)
    {
        perror("write");
        return -1;
    }

    // Fetch and save file
    ssize_t total_bytes_read = fetch_and_save_file(config);

    close(sockfd);
    sockfd = -1;

    if (total_bytes_read < 0) return -1;

    print_statistics(total_bytes_read, start_time);

    return 0;
}

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    struct addrinfo *server_info;
    Config config;
    if (parse_args(argc, argv, &server_info, &config) != 0) return -1;
    return run(server_info, &config);
}