#include "arg_checkers.h"
#include "request_codec.h"
#include "socket_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define REQUIRED_ARGC 5
#define MAX_LISTEN_NUM 5

int sockfd_half = -1;

void tear_down()
{
    if (close(sockfd_half) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

static void sigint_handler(int _)
{
    tear_down();
    exit(EXIT_SUCCESS);
}
typedef struct
{
    uint16_t secret_key;
    unsigned long blocksize_byte;
} Config;

int parse_args(int argc, char *argv[], struct addrinfo **server_info,
               Config *const config)
{
    if (argc < REQUIRED_ARGC)
    {
        fprintf(stderr,
                "Usage: %s <server-ip> <server-port> <secret-key> "
                "<blocksize-byte>\n",
                argv[0]);
        return -1;
    }

    int status;

    if ((status = build_addrinfo(server_info, argv[1], argv[2], SOCK_STREAM)) !=
        0)
        return status;

    long long secret_key = strtoull(argv[3], NULL, 0);
    if ((status = check_secret_key(secret_key) != 0)) return status;
    config->secret_key = (uint16_t)secret_key;

    config->blocksize_byte = strtoul(argv[4], NULL, 0);

    return 0;
}

int sanitize_client_addr(const struct sockaddr *const addr)
{
    char ipstr[INET_ADDRSTRLEN];
    if (!inet_ntop(addr->sa_family, &((struct sockaddr_in *)addr)->sin_addr,
                   ipstr, sizeof(ipstr)))
    {
        perror("inet_ntop");
        return -1;
    }

    // Allow only 128.10.25.* or 128.10.112.*
    const char *const allowed_ips[] = {"128.10.25.", "128.10.112."};
    for (size_t i = 0; i < sizeof(allowed_ips) / sizeof(allowed_ips[0]); i++)
        if (strncmp(ipstr, allowed_ips[i], strlen(allowed_ips[i])) == 0)
            return 0;

    fprintf(stderr, "client address denied: %s", ipstr);
    return -1;
}

int read_request(int sockfd_full, char *const filename,
                 uint16_t *const secret_key)
{
    uint8_t request[REQUEST_SIZE];
    const ssize_t bytes_read = read(sockfd_full, request, REQUEST_SIZE);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    decode_request(request, filename, secret_key);
    return 0;
}

int send_file(int sockfd_full, const char *const filename,
              uint16_t blocksize_byte)
{
    FILE *const file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }

    int8_t buffer[blocksize_byte];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, sizeof(int8_t), sizeof(buffer), file)) >
           0)
    {
        if (write(sockfd_full, buffer, bytes_read) == -1)
        {
            perror("write");
            break;
        }
    }

    if (fclose(file) == -1)
    {
        perror("fclose");
        return -1;
    }

    if (ferror(file))
    {
        perror("fread");
        return -1;
    }

    return 0;
}

int run(const Config *const config)
{
    while (1)
    {
        // Accept connection
        int sockfd_full = -1;
        struct sockaddr client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if ((sockfd_full =
                 accept(sockfd_half, &client_addr, &client_addr_len)) == -1)
        {
            perror("accept");
            continue;
        }

        // TODO: Enable when tested on lab machines.
        // if (sanitize_client_addr(&client_addr) == -1)
        // {
        //     if (close(sockfd_full) == -1) perror("close");
        //     continue;
        // }

        uint16_t secret_key;
        char filename[MAX_FILENAME_LEN];
        if (read_request(sockfd_full, filename, &secret_key) != 0)
        {
            if (close(sockfd_full) == -1) perror("close");
            continue;
        }

        if (config->secret_key != secret_key)
        {
            fprintf(stderr, "Secret key mismatch: %hu != %hu\n",
                    config->secret_key, secret_key);
            if (close(sockfd_full) == -1) perror("close");
            continue;
        }

        if (check_filename(filename) != 0)
        {
            if (close(sockfd_full) == -1) perror("close");
            continue;
        }

        int toss_coin = rand() % 2;
        if (toss_coin == 0) // Ignore command
        {
            fprintf(stderr, "Toss a coin and ignore request\n");
            if (close(sockfd_full) == -1) perror("close");
            continue;
        }

        // Check if file exists
        if (access(filename, F_OK) == -1)
        {
            perror("access");
            if (close(sockfd_full) == -1) perror("close");
            continue;
        }

        fflush(stdout); // Flush stdout before forking
        const pid_t k = fork();
        if (k == 0)
        {
            // Child process
            send_file(sockfd_full, filename, config->blocksize_byte);
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent process
            int status;
            if (waitpid(k, &status, 0) == -1) perror("waitpid");
            if (close(sockfd_full) == -1) perror("close");
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    const struct sigaction sigint_action = {.sa_handler = sigint_handler};
    sigaction(SIGINT, &sigint_action, NULL);

    struct addrinfo *info;
    Config config;

    if (parse_args(argc, argv, &info, &config) != 0) return -1;

    // bind to the socket
    if ((bind_socket_with_first_usable_addr(info, sockfd_half)) == -1)
    {
        if (close(sockfd_half) == -1) perror("close");
        return -1;
    }

    // listen to the socket
    if ((listen(sockfd_half, MAX_LISTEN_NUM)) == -1)
    {
        perror("listen");
        if (close(sockfd_half) == -1) perror("close");
        return -1;
    }

    int status = run(&config);

    tear_down();

    return status;
}