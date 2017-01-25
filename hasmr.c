#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <libzbc/zbc.h>

int smr_open(char *path,struct zbc_device *dev);
int smr_close(struct zbc_device *dev);

int main()
{
	int ret;
	struct zbc_device *dev;
	char path[64]="/dev/sdb";
	
	ret=smr_open(path,dev);
	
	if(ret != 1)
	{
		printf("OPEN Failure..\n");
		exit(-1);
	}
	
	ret=smr_close(dev);
	if(ret != 1)
	{
		printf("CLOSE Failure..\n");
		exit(-1);
	}


	return 1;
}



int smr_open(char *path,struct zbc_device *dev)
{
	int ret = 1;
	struct zbc_device_info info;
    enum zbc_reporting_options ro = ZBC_RO_ALL;
    
    ret=zbc_open(path,O_RNONLY,&dev);
    if(ret != 0)
    {
    	printf("Open Device Failure!\n");
    	return -1;
    }
    
    ret=zbc_get_device_info(dev,&info);
    if(ret < 0)
    {
    	printf("Get Device Info Failure!\n");
    	return -1;
    }
    
    printf("Device %s: %s\n",path,info.zbd_vendor_id);
    printf("    %s interface, %s disk model\n",zbc_disk_type_str(info.zbd_type),zbc_disk_model_str(info.zbd_model));
    printf("    %llu logical blocks of %u B\n",(unsigned long long) info.zbd_logical_blocks,(unsigned int) info.zbd_logical_block_size);
    printf("    %llu physical blocks of %u B\n",(unsigned long long) info.zbd_physical_blocks,(unsigned int) info.zbd_physical_block_size);
    printf("    %.03F GB capacity\n",(double) (info.zbd_physical_blocks * info.zbd_physical_block_size) / 1000000000);

	return 1;
}


int smr_close(struct zbc_device *dev)
{
	if(dev)
	{
		zbc_close(dev);
		return 1;
	}
	printf("CLOSE Failure..\n");
	return -1;	
}









