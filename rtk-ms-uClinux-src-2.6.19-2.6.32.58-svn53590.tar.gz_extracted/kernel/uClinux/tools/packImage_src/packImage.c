#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define PADDING_BYTE  (0xFF)

typedef unsigned int uint32;

typedef struct image_region_s{
	char name[512];
	uint32 len;
	uint32 realsize;
	struct image_region_s *next;
}image_region_t;

image_region_t region_h;

void usage(char *execName){
		printf("Usage:");
		printf("\t%s outputFile imageName1 imageRange1 imageName2 imageRange2 ... \n", execName);
}

image_region_t *getImage_region(void){
	image_region_t *p;
	p = (image_region_t *) malloc(sizeof(image_region_t));
	if( NULL==p ){
		printf("malloc fail!!!\n");
		exit(1);
	}
	p->next = NULL;
	p->len = 0;
	return p;
}

int fillPad(const unsigned char pad_byte, int fd, uint32 len){
	int i;
	unsigned char *buf;
	uint32 wrote;

	buf = (char *) malloc(len);
	if( NULL==buf ){
		printf("malloc fail for %s\n", __FUNCTION__);
		return -1;
	}

	for(i=0;i<len;i++){
		buf[i] = pad_byte;
	}

	wrote = 0;
	do{
		wrote += write(fd, buf, len);
	}while(wrote<len);

	free(buf);

	return 0;
}

#define READ_BUFF_SIZE  (8*1024*1024)
int fillImage(image_region_t *head, char *filename){
	int fd, fd_out;
	int retval;
	uint32 read_bytes;
	uint32 offset;
	image_region_t *iter;
	unsigned char buf[READ_BUFF_SIZE];

	if( NULL==head ){
		iter = &region_h;
		if( NULL==iter->next ){
			printf("No Input File Error\n");
			return -1;
		}else{
			iter = iter->next;
		}
	}

	if( NULL==filename ){
		printf("Input Filename Error(NULL PTR)\n");
		return -1;
	}

	fd_out = creat(filename, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	if( -1==fd_out ){
		printf("Open %s as output file fail!\n", filename);
		return -1;
	}

	for(;iter;iter=iter->next){
		fd = open(iter->name, O_RDONLY);
		if( -1==fd ){
			printf("Open %s as input file fail!\n", iter->name);
			return -1;
		}
		offset = 0;
		while ( 0!=(read_bytes = read(fd, buf, READ_BUFF_SIZE)) ){
			offset += read_bytes;
			retval = write(fd_out, buf, read_bytes);
		}
		if( NULL!=iter->next ){ 
			fillPad(PADDING_BYTE, fd_out, (iter->len) - offset );
		}
		close(fd);
	}

}

// No error check
// The address of input image sequence must be in order
int main(int argc, char *argv[]){
	int i, j;
	image_region_t *iter = &region_h;
	struct stat st;

	iter->next = NULL;

	if( argc<3 || 1==(argc%2) ){
		usage(argv[0]);
		return -1;
	}

	for(i=2;i<argc;i+=2){
		iter->next = getImage_region();
		iter = iter->next;
		strncpy( (char*) iter->name, (char*) argv[i], sizeof(iter->name));
		sscanf(argv[i+1], "0x%x", &(iter->len) );

		stat(iter->name, &st);
		if( st.st_size > iter->len ){
			printf("File %s size(0x%08x) is larger than it's range(0x%08x)\n", \
					iter->name, st.st_size, iter->len);
			return -1;
		}
		iter->realsize = st.st_size;

	}

	if ( 0!=fillImage(NULL, argv[1]) ){
		printf("Pack Image Fail!!!\n");
		return -1;
	}else{
		uint32 offset = 0;
		for(iter=region_h.next;iter;iter=iter->next){
			printf("[Name:%s]\n", iter->name);
			printf("\t[0x%08x-0x%08x][  IMAGE]\n", offset, offset + iter->realsize - 1);
			if( NULL!=iter->next ){
				printf("\t[0x%08x-0x%08x][PADDING]\n", \
						offset + iter->realsize, offset + iter->len - 1);
			}else{
				printf("\t[ No padding for the last image]\n");
			}
			offset += iter->len;
		}
		return 0;
	}

}

