#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include "large_freader.h"
#include "devide_fwriter.h"

#define WRITE_SIZE 2048

static int device_fnc(size_t current_write_len) {
//	printf("%s check %d\n", __FUNCTION__, (int)current_write_len);
	return WRITE_SIZE<current_write_len;
}

//1 time read size
#define READ_SIZE (1536)
static int write_size() {
	int ret=0;
	while( !device_fnc(ret) ) {
		ret += READ_SIZE;
	}
	return ret;
}

#define MD5SUM_LEN 32

static char * hash_result(char *dir, char *prefix, int index) {
	char cmd[128];
	sprintf(cmd, "md5sum %s/%s_%06d", dir, prefix, index);
	FILE *fp = popen(cmd, "r");
	char *result=(char *)malloc(128);
	fgets(result, 128, fp);
	pclose(fp);
//	printf("result:%s\n", result);
	return result;
}

static int check_hash(char *dir1, char *dir2, char *prefix, int index) {
	char *res1=hash_result(dir1, prefix, index);
	char *res2=hash_result(dir2, prefix, index);
	int result= memcmp(res1, res2, MD5SUM_LEN)==0;
	free(res1);
	free(res2);
}

#define TMPFILE_SIZE (2*1024*1024) //2MByte
#define READ_DEVIDE_SIZE (10*1024) //2MByte

static int check_all(char *dir1, char *dir2, char *prefix) {
	int filenum = TMPFILE_SIZE/write_size();
	int i=0;
	for(i=0;i<filenum;i++) {
		if(!check_hash(dir1, dir2, prefix, i)) {
			printf("hash[%d] is different!!\n", i);
			return 0;
		}
	}
	return 1;
}

int main() {
	//create tmp file
	char tmpfile[]="2mdata";
	char cmd[128];
	sprintf(cmd, "head -c %d /dev/urandom > %s", TMPFILE_SIZE, tmpfile);
	system(cmd);

	//for write dir
	char writedir[]="tmpdir";
	char prefix[]="test";
	mkdir(writedir, 0777);

#if 0
	system("free");
	int cnt=0;
	for(cnt=0;cnt<10;cnt++) {
#endif
	//writer handler
	void * write_handle = devide_fwriter_open(writedir, prefix, device_fnc);

	//read handler
	void * read_handle = large_freader_open(tmpfile, READ_DEVIDE_SIZE);
	int ret=0;
	char buffer[WRITE_SIZE];
	//copy to write dir
	while(1) {
		ret = large_freader_read(buffer,READ_SIZE,read_handle);
		if(ret != -1) {
			devide_fwriter_write(buffer,ret,write_handle);
		}

		//finish to read
		if(ret != READ_SIZE) {
			break;
		}
	}

	//close
	large_freader_close(read_handle);
	devide_fwriter_close(write_handle);
#if 0
	system("free");
	}
#endif
	char tmpdir[]="tmp2";
	//check devide data
	mkdir(tmpdir, 0777);
	sprintf(cmd, "/usr/bin/split -d --suffix-length=6 -b %d %s %s/%s_", write_size(), tmpfile, tmpdir, prefix);
	system(cmd);

	//check hash
	if(!check_all(writedir, tmpdir, prefix)) {
		fprintf(stderr, "failed to read/write func!!\n");
		return -1;
	}

	unlink(tmpfile);
	sprintf(cmd, "rm -rf %s", tmpdir);
	system(cmd);
	sprintf(cmd, "rm -rf %s", writedir);
	system(cmd);
	return 0;
}
