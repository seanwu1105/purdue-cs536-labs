#if !defined(_PARAMETER_CHECKERS_H_)
#define _PARAMETER_CHECKERS_H_

#define MAX_FILENAME_LEN 8
#define MIN_BLOCKSIZE 1
#define MAX_BLOCKSIZE 65535
#define MIN_PACKETS_PER_SECOND 1

int check_filename(const char *const filename);
int check_blocksize(const unsigned long long blocksize);
int check_packets_per_second(const unsigned long long packets_per_second);

#endif // _PARAMETER_CHECKERS_H_