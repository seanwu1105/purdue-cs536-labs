#if !defined(_ARG_CHECKERS_H_)
#define _ARG_CHECKERS_H_

#define MIN_SECRET_KEY 0
#define MAX_SECRET_KEY 65535

int check_filename(const char *const filename);
int check_secret_key(const long long secret_key);

#endif // _ARG_CHECKERS_H_