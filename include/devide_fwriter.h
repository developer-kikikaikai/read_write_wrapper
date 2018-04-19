#ifndef DEVIDE_FWITER_
#define DEVIDE_FWITER_
#include <stdio.h>
typedef int (*devide_fwiter_gonext)(size_t current_write_len);

void * devide_fwriter_open(char *path, char *prefix, devide_fwiter_gonext gonext_fnc);
size_t devide_fwriter_write(const void *ptr, size_t size, void *stream);
void devide_fwriter_close(void *stream);
#endif
