#if !defined(_PARAMETER_CHECKERS_H_)
#define _PARAMETER_CHECKERS_H_

#define MAX_FILENAME_LEN 8
#define MIN_BLOCKSIZE 1
#define MAX_BLOCKSIZE 65535

int check_filename(const char *const filename);
int check_blocksize(const long long blocksize);

#endif // _PARAMETER_CHECKERS_H_