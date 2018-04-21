#include<stdlib.h>
#include<string.h>
#include <errno.h>
#include "large_freader.h"

#define TMPDIR "/tmp/large_freader_devicefile"
#define FPREFIX "tmpfile"
#define FNAME_MAX (128)
#define CMD_MAX (1024)
#define DBGFLAG
#ifdef DBGFLAG
#define READ_DEBUG_PRINT(...)  READ_DEBUG_PRINT_(__VA_ARGS__, "")
#define READ_DEBUG_PRINT_(fmt, ...)  \
        printf("%s(%d): "fmt"%s", __FUNCTION__,__LINE__, __VA_ARGS__)
#else
#define READ_DEBUG_PRINT(...) 
#endif

//handle state check macro
#define IS_DEVIDE_FILE(handle) ((handle)->tmpdir[0]!=0)
#define IS_LAST_FILE(handle) ((handle)->cur_index>=(handle)->max_index)

//handle
struct large_freader_s{
	FILE *fp;
	int cur_index;
	int max_index;
	char tmpdir[FNAME_MAX];
};
//private file manage function for handle
static inline int get_current_dirname(struct large_freader_s *handle, char *name, size_t len);
static inline void get_current_fname(struct large_freader_s *handle, char *name, size_t len);

static void remove_current_file(struct large_freader_s *handle);
static void remove_devided_file(struct large_freader_s * handle);
static FILE * freader_fopen(struct large_freader_s *handle);
static void freader_fclose(struct large_freader_s *handle);

static void separate_file(const char *path, size_t maxsize,  struct large_freader_s *handle);
static void get_tmpname(char *name, size_t len);
static unsigned long get_size(const char *path);
static void call_cmd(char *cmd, char *result, size_t result_len);

static void get_tmpname(char *name, size_t len) {
	snprintf(name, len, "%s_%d", TMPDIR, time(NULL));
}

static inline int get_current_dirname(struct large_freader_s *handle, char *name, size_t len) {
	return snprintf(name, len, "%s/%s", handle->tmpdir, FPREFIX);
}

static inline void get_current_fname(struct large_freader_s *handle, char *name, size_t len) {
	int tmplen=get_current_dirname(handle, name, len);
	//add index
	snprintf(&name[tmplen], len-tmplen, "%06d", handle->cur_index);
}

static void call_cmd(char *cmd, char *result, size_t result_len) {
	READ_DEBUG_PRINT("call cmd:%s\n", cmd);
	FILE *fp=popen(cmd, "r");
	if(!fp) {
		return;
	}
	if(result) {
		if(fgets(result, result_len, fp)==NULL) {
			READ_DEBUG_PRINT("failed to read:%s\n",strerror(errno) );
		}
		READ_DEBUG_PRINT("result:%s\n", result);
	}

	pclose(fp);
}

#define SKIP_SPACE_NUM (4)
static unsigned long get_size(const char *path) {
	char cmd[CMD_MAX];
	char result[CMD_MAX];
	char *result_p=result;
	unsigned long ret=0;

	snprintf(cmd, sizeof(cmd), "ls -l %s", path);
	call_cmd(cmd, result, CMD_MAX);
	int i=0;
	for(i=0;i<SKIP_SPACE_NUM;i++) {
		result_p=strstr(result_p," ")+1;
	}
	ret = strtoul(result_p, NULL, 10);
	READ_DEBUG_PRINT("result:%lu\n", ret);
	return ret;
}

static void separate_file(const char *path, unsigned long maxsize,  struct large_freader_s *handle) {
	char cmd[CMD_MAX]={0};
	FILE *fp =NULL;
	READ_DEBUG_PRINT("enter\n");

	//get tmp dir
	get_tmpname(handle->tmpdir, FNAME_MAX);
	//it's better to fix permission
	mkdir(handle->tmpdir, 0777);

	//separate file
	char name[FNAME_MAX];
	get_current_dirname(handle, name, FNAME_MAX);
	snprintf(cmd, sizeof(cmd), "/usr/bin/split -d --suffix-length=6 -b %lu %s %s", maxsize, path, name);

	call_cmd(cmd,NULL,0);

	//get file num
	snprintf(cmd, sizeof(cmd), "/bin/ls %s/* | wc -l", handle->tmpdir);
	char result[CMD_MAX]={0};
	//get file num
	call_cmd(cmd, result, CMD_MAX);
	handle->max_index = atoi(result);
}

static FILE * freader_fopen(struct large_freader_s *handle) {
	char dname[FNAME_MAX];
	get_current_fname(handle, dname, FNAME_MAX);
	READ_DEBUG_PRINT("open %s\n", dname);
	return fopen(dname, "r");
}

static void freader_fclose(struct large_freader_s *handle) {
	//close file
	if(!handle->fp) {
		return;
	}

	fclose(handle->fp);
	handle->fp=NULL;

	//remove tmpfile
	if(IS_DEVIDE_FILE(handle)) {
		remove_current_file(handle);
	}
}

static void remove_current_file(struct large_freader_s *handle) {
	char dname[FNAME_MAX];
	get_current_fname(handle, dname, FNAME_MAX);
	READ_DEBUG_PRINT("open %s\n", dname);
	unlink(dname);
}

static void remove_devided_file(struct large_freader_s * handle) {
	while(!IS_LAST_FILE(handle)) {
		remove_current_file(handle);
		handle->cur_index++;
	}
	rmdir(handle->tmpdir);
}

void * large_freader_open(const char *path, unsigned long maxsize) {
	//use handle
	READ_DEBUG_PRINT("enter\n");
	if(!path || maxsize == 0) {
		READ_DEBUG_PRINT("input error\n");
		return NULL;
	}

	struct large_freader_s * handle = (struct large_freader_s *)malloc(sizeof(struct large_freader_s));
	if(!handle) {
		READ_DEBUG_PRINT("malloc error:%s\n",strerror(errno) );
		return NULL;
	}

	memset(handle, 0, sizeof(struct large_freader_s));

	//check size
	unsigned long fsize = get_size(path);
	if(fsize<=maxsize) {
		//same as fopen
		handle->fp=fopen(path, "r");
		handle->max_index=1;
	} else {
		//separate file and open file as order
		separate_file(path, maxsize, handle);
		handle->fp = freader_fopen(handle);
	}
	if(!handle->fp) {
		goto err;
	}
	return handle;

err:
	if(handle) {
		free(handle);
	}
	return NULL;
}

int large_freader_is_devide(void * stream) {
	READ_DEBUG_PRINT("enter\n");
	struct large_freader_s * handle = (struct large_freader_s *) stream;
	//fail safe
	if(!handle) {
		return 0;
	}

	return IS_DEVIDE_FILE(handle);
}

size_t large_freader_read(void * prt, size_t size,void * stream) {
	READ_DEBUG_PRINT("enter\n");
	struct large_freader_s * handle = (struct large_freader_s *) stream;
	//fail safe
	if(!handle || !handle->fp) {
		READ_DEBUG_PRINT("input error\n");
		return -1;
	}

	size_t ret = fread(prt, 1, size, handle->fp);
	if(ret == size) {
		//read success, return normaly
		return ret;
	}

	freader_fclose(handle);
	//move to next
	handle->cur_index++;

	if(IS_LAST_FILE(handle)) {
		//finish to read
		return ret;
	}

	//open next
	handle->fp = freader_fopen(handle);
	if(handle->fp) {
		ret += fread(((char *)prt)+ret, 1, size-ret, handle->fp);
	}

	return ret;
}

void large_freader_close(void *stream) {
	READ_DEBUG_PRINT("enter\n");
	struct large_freader_s * handle = (struct large_freader_s *) stream;
	//fail sase
	if(!handle) {
		READ_DEBUG_PRINT("input error\n");
		return;
	}

	freader_fclose(handle);
	//move to next
	handle->cur_index++;

	//if devide file, remove dir
	if(IS_DEVIDE_FILE(handle)) {
		remove_devided_file(handle);
	}
	free(handle);
}
