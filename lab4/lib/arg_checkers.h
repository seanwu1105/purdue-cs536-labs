#if !defined(_ARG_CHECKERS_H_)
#define _ARG_CHECKERS_H_

#define MAX_FILENAME_LEN 8
#define MIN_SECRET_KEY 0
#define MAX_SECRET_KEY 65535
#define MIN_BLOCKSIZE 1
#define MAX_BLOCKSIZE 1471
#define MIN_WINDOWSIZE 1
#define MAX_WINDOWSIZE 64

int check_filename(const char *const filename);
int check_secret_key(const long long secret_key);
int check_blocksize(const long long blocksize);
int check_windowsize(const long long windowsize);

#endif // _ARG_CHECKERS_H_