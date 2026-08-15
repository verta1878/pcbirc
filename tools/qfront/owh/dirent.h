#ifndef _DIRENT_H
#define _DIRENT_H
struct dirent { char d_name[256]; };
typedef struct { int _fd; } DIR;
DIR *opendir(const char *name);
struct dirent *readdir(DIR *d);
int closedir(DIR *d);
#endif
