//using aio_write to handle writes to HA-SMR drives
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <errno.h>
#include <pthread.h>

#define MEM_ALIGN				512  // Memory alignment
#define BYTE_PER_BLOCK			512 
#define LARGEST_REQUEST_SIZE	10000  // Largest request size in blocks

int main()
{
	int i,ret,fd;
	struct aiocb *cb;
	char *buf;
    char path[64]="/dev/sdb";
	
	if (posix_memalign((void**)&buf, MEM_ALIGN, LARGEST_REQUEST_SIZE * BYTE_PER_BLOCK))
	{
		printf("Error allocating buffer \n");
		exit(-1);
	}
	
	for(i=0;i<LARGEST_REQUEST_SIZE*BYTE_PER_BLOCK;i++)
	{
		buf[i]=(char)(rand()%26+65);
	}

	fd=open(path,O_DIRECT | O_SYNC | O_RDWR);
	if(fd<0)
	{
		printf("Open Device Error!\n");
		exit(-1);
	}
	
    cb=(struct aiocb *)malloc(sizeof(struct aiocb));
    memset(cb,0,sizeof(struct aiocb));
	cb->aio_fildes=fd;
	cb->aio_buf=buf;
	cb->aio_nbytes=512;
	cb->aio_offset=10240;
	
	ret=aio_write(cb);
	if(ret<0)
	{
		printf("aio_write error!\n");
		exit(-1);
	}
	
	close(fd);

	return 1;
}
