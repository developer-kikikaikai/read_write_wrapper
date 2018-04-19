#ifndef LARGE_FREADER__
#define LARGE_FREADER_
#include<stdio.h>
//fix open read only
//open reader
void * large_freader_open(const char *path, size_t maxsize);
//if devide file?
int large_freader_is_devide(void * stream);
//read file by using reader. stream if returned at large_freader_open
size_t large_freader_read(void * prt, size_t size, void * stream);
//close reader, stream if returned at large_freader_open
void large_freader_close(void *stream);
#endif
