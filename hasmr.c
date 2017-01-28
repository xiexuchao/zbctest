#define _GNU_SOURCE
#include <libzbc/zbc.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <aio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <pthread.h>

#define MEM_ALIGN				512  // Memory alignment
#define BYTE_PER_BLOCK			512 
#define LARGEST_REQUEST_SIZE	2048 // Largest request size in blocks

struct smr_info{
    char *path;
    int fd;
    struct aiocb *cb;
    char *buf;
    long long wp;
    struct zbc_device *dev;
    unsigned int nr_zones;
};

struct req_info{
	long long time;
    long long lba;
    unsigned int type;				//0->Read,1->Write
    unsigned int size;
};

struct smr_info *smr_open(char *path,struct smr_info *smr);
int smr_close(struct smr_info *smr);
struct smr_info *smr_report_wp(struct smr_info *smr,unsigned int index);
struct smr_info *smr_write(struct smr_info *smr,unsigned int zone,long long lba,unsigned int size);
struct smr_info *smr_write(struct smr_info *smr,struct req_info *req);

int main()
{
	int i,ret;
	char path[64]="/dev/sdb";
    struct smr_info *smr;
	
    smr=(struct smr_info *)malloc(sizeof(struct smr_info));
    if (!smr) 
    {
		printf("smr malloc Error!\n");
		exit(-1);
    }
    memset(smr, 0, sizeof(struct smr_info));
    
    req=(struct req_info *)malloc(sizeof(struct req_info));
    if(!req)
    {
    	printf("req malloc Error!\n");
		exit(-1);
    }
    memset(req, 0, sizeof(struct req_info));
    
	smr=smr_open(path,smr);
	
	req->lba=11111;
	req->size=8;
	smr=smr_write(smr,req);
	
	smr=smr_report_wp(smr,50);
    printf("wp=%lld \n",smr->wp);
    smr=smr_report_wp(smr,1000);
    printf("wp=%lld \n",smr->wp);
	smr=smr_report_wp(smr,500);
    printf("wp=%lld \n",smr->wp);
    
    smr=smr_write(smr,req);
	
	ret=smr_close(smr);

	return 1;
}



struct smr_info *smr_open(char *path,struct smr_info *smr)
{
	int ret = 1;
	struct zbc_device_info info;
    enum zbc_reporting_options ro = ZBC_RO_ALL;

	/*
	**	open as normal block device
	*/
	//smr->path
	smr->path=path;
	//smr->fd
	smr->fd=open(smr->path, O_DIRECT | O_SYNC | O_RDWR);
	if(smr->fd<0)
	{
		printf("Open Device open() Error!\n");
		exit(-1);
	}
	//smr->cb
	smr->cb=(struct aiocb *)malloc(sizeof(struct aiocb));
    memset(smr->cb,0,sizeof(struct aiocb));	
	//smr->buf
	if (posix_memalign((void**)&(smr->buf), MEM_ALIGN, LARGEST_REQUEST_SIZE * BYTE_PER_BLOCK))
	{
		printf("Error allocating buffer \n");
		exit(-1);
	}
	for(i=0;i<LARGEST_REQUEST_SIZE*BYTE_PER_BLOCK;i++)
	{
		smr->buf[i]=(char)(rand()%26+65);
	}
	//smr->wp	
	smr->wp=-1;

	/*
	**	open as zoned block device
	*/
	//smr->dev
    ret=zbc_open(smr->path,O_RDONLY,&smr->dev);
    if(ret != 0)
    {
    	printf("Open Device Failure!\n");
    	exit(-1);
    }
    //smr->nr_zones
    ret = zbc_report_nr_zones(smr->dev, lba, ro, &nr_zones);
    if ( ret != 0 )
    {
    	printf("Report Number of Zones ERROR!\n");
    	exit(-1);
    }
    printf("nr_zones=%d\n",nr_zones);
    //smr->dev->info
    ret=zbc_get_device_info(smr->dev,&info);
    if(ret < 0)
    {
    	printf("Get Device Info Failure!\n");
    	exit(-1);
    }
    
    printf("Device %s: %s\n",smr->path,info.zbd_vendor_id);
    printf("    %s interface, %s disk model\n",zbc_disk_type_str(info.zbd_type),zbc_disk_model_str(info.zbd_model));
    printf("    %llu logical blocks of %u B\n",(unsigned long long) info.zbd_logical_blocks,(unsigned int) info.zbd_logical_block_size);
    printf("    %llu physical blocks of %u B\n",(unsigned long long) info.zbd_physical_blocks,(unsigned int) info.zbd_physical_block_size);
    printf("    %.03F GB capacity\n",(double) (info.zbd_physical_blocks * info.zbd_physical_block_size) / 1000000000);
    
	return smr;
}


int smr_close(struct smr_info *smr)
{
	if(smr->dev)
	{
		close(smr->fd);
		zbc_close(smr->dev);
		free(smr->cb);
		free(smr->buf);
		free(smr);
        printf("CLOSE SUCCESS!\n");
		return 1;
	}
	printf("CLOSE Failure..\n");
	return -1;	
}


struct smr_info *smr_report_wp(struct smr_info *smr,unsigned int index)
{
	int ret;
	unsigned long long lba=0;
    enum zbc_reporting_options ro=ZBC_RO_ALL;
    struct zbc_zone *z, *zones=NULL;
       
    if(index > smr->nr_zones)
    {
    	printf("Index Error!\n");
    	exit(-1);
    }else if(index < 64)
    {
    	printf("Convential Zone Index!\n");
    	exit(-1);
    }
    
    zones=(zbc_zone_t *)malloc(sizeof(zbc_zone_t)*nr_zones);
    if (!zones) 
    {
		printf("malloc Error!\n");
		exit(-1);
    }
    memset(zones, 0, sizeof(zbc_zone_t)*smr->nr_zones);
    
    ret=zbc_report_zones(smr->dev,lba,ro,zones,&smr->nr_zones);
    if(ret!=0)
    {
    	printf("Report Zone Failure!\n");
		exit(-1);
    }
    
    z=&zones[index];
    printf("Zone %05d: type 0x%x (%s), cond 0x%x (%s), need_reset %d, non_seq %d, LBA %llu, %llu sectors, wp %llu\n",
       			   index,
			       zbc_zone_type(z),
                   zbc_zone_type_str(zbc_zone_type(z)),
                   zbc_zone_condition(z),
                   zbc_zone_condition_str(zbc_zone_condition(z)),
                   zbc_zone_need_reset(z),
                   zbc_zone_non_seq(z),
                   zbc_zone_start_lba(z),
                   zbc_zone_length(z),
                   zbc_zone_wp_lba(z));
    smr->wp=zbc_zone_wp_lba(z);  
	
    if(zones)
    {
        free(zones);
    }
    return smr;
}

struct smr_info *smr_write(struct smr_info *smr,struct req_info *req)
{
	int ret;
	int zone_id;
	
	zone_id=req->lba/(256*1024*2);
	
	smr=smr_report_wp(smr,zone_id);
	if(req->lba == smr->wp)
	{
		//this is a sequential write
		printf("this is a sequential write!\n");
	}
	smr->cb->aio_fildes=smr->fd;
	smr->cb->aio_buf=smr->buf;
	smr->cb->aio_nbytes=req->size*512;
	smr->cb->aio_offset=(long long)req->lba*512;
	
	ret=aio_write(cb);
	if(ret)
	{
		printf("aio_write error!\n");
		exit(-1);
	}
	return smr;
	
}









