#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "access_control.h"
#include "bbcodec.h"

#define BUFFER_SIZE 1024

ssize_t load_public_keys(PublicKey public_keys[])
{
    FILE *file = fopen("acl.dat", "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }
    char content[BUFFER_SIZE];
    const size_t bytes_read =
        fread(content, sizeof(char), sizeof(content), file);
    fclose(file);
    content[bytes_read] = '\0';
    if (bytes_read == 0) return 0;

    size_t public_key_idx = 0;

    const char *val = strtok(content, " \t\n\0");
    if (val == NULL) return 0;
    strncpy(public_keys[public_key_idx].ip, val, INET_ADDRSTRLEN);
    val = strtok(NULL, " \t\n\0");
    if (val == NULL)
    {
        fprintf(stderr, "Unbalanced ACL file\n");
        return -1;
    }
    public_keys[public_key_idx].pubkey = (uint32_t)strtoul(val, NULL, 10);
    public_key_idx++;

    while (1)
    {
        val = strtok(NULL, " \t\n\0");
        if (val == NULL) break;
        strncpy(public_keys[public_key_idx].ip, val, INET_ADDRSTRLEN);

        val = strtok(NULL, " \t\n\0");
        if (val == NULL)
        {
            fprintf(stderr, "Unbalanced ACL file\n");
            return -1;
        }
        public_keys[public_key_idx].pubkey = (uint32_t)strtoul(val, NULL, 10);
        public_key_idx++;
    }
    return public_key_idx;
}

int check_access(const struct sockaddr *const addr, const uint32_t certificate,
                 const PublicKey public_keys[], const size_t public_keys_size)
{
    uint32_t ip = ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
    for (size_t i = 0; i < public_keys_size; i++)
    {
        if (bbencode(certificate, public_keys[i].pubkey) == ip) return 1;
    }
    return 0;
}