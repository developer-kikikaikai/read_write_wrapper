#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "devide_fwriter.h"

#define FNAME_MAX (128)
#ifdef DBGFLAG
#define WRITE_DEBUG_PRINT(...)  WRITE_DEBUG_PRINT_(__VA_ARGS__, "")
#define WRITE_DEBUG_PRINT_(fmt, ...)  \
        printf("%s(%d): "fmt"%s", __FUNCTION__,__LINE__, __VA_ARGS__)
#else
#define WRITE_DEBUG_PRINT(...) 
#endif

//handle
struct devide_fwriter_s {
	FILE *fp;
	char dirname[FNAME_MAX];
	int index;
	unsigned long writelen;
	devide_fwiter_gonext gonext;
};

static void devide_fwriter_fopen(struct devide_fwriter_s *handle) {
	char dname[FNAME_MAX];
	snprintf(dname, FNAME_MAX, "%s_%06d", handle->dirname, handle->index);
	WRITE_DEBUG_PRINT("open %s\n", dname);
	handle->fp = fopen(dname, "w");
}

static void devide_fwriter_fclose(struct devide_fwriter_s *handle) {
	fclose(handle->fp);
	handle->fp=NULL;
	handle->writelen = 0;
	handle->index++;
}

void * devide_fwriter_open(char *dir, char *prefix, devide_fwiter_gonext gonext_fnc) {
	WRITE_DEBUG_PRINT("enter\n");
	//fail safe
	if(!dir || !prefix) {
		WRITE_DEBUG_PRINT("input err\n");
		return NULL;
	}

	struct devide_fwriter_s *handle = malloc(sizeof(struct devide_fwriter_s));
	if(!handle) {
		WRITE_DEBUG_PRINT("malloc error:%s\n",strerror(errno) );
		return NULL;
	}

	memset(handle, 0, sizeof(struct devide_fwriter_s));
	//keep dir name
	snprintf(handle->dirname, sizeof(handle->dirname), "%s/%s", dir, prefix);
	handle->gonext = gonext_fnc;
	return handle;
}

size_t devide_fwriter_write(const void *ptr, size_t size, void *stream) {
	WRITE_DEBUG_PRINT("enter\n");
	struct devide_fwriter_s *handle = (struct devide_fwriter_s *)stream;
	//fail safe
	if(!ptr || size==0 || !handle) {
		WRITE_DEBUG_PRINT("input err\n");
		return -1;
	}

	//open file at write, to care extra open (without write data)
	if(!handle->fp) {
		devide_fwriter_fopen(handle);
		if(!handle->fp) {
			WRITE_DEBUG_PRINT("fopen error:%s\n", strerror(errno));
			return -1;
		}
	}

	size_t ret = fwrite(ptr, size, 1, handle->fp);
	//error check same as 
	if(ret != 1) {
		return ret;
	}

	handle->writelen += size;

	//close check
	if(handle->gonext && handle->gonext(handle->writelen)) {
		devide_fwriter_fclose(handle);
	}
	return ret;
}

void devide_fwriter_close(void *stream) {
	WRITE_DEBUG_PRINT("enter\n");
	struct devide_fwriter_s *handle = (struct devide_fwriter_s *)stream;
	if(!handle) {
		WRITE_DEBUG_PRINT("input err\n");
		return;
	}

	if(handle->fp) {
		fclose(handle->fp);
	}

	free(handle);
}
