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
    
    ret=zbc_open(path,O_RDONLY,&dev);
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


int smr_report(struct zbc_device *dev,unsigned int index)
{
	int ret;
	unsigned long long lba=0;
    enum zbc_reporting_options ro=ZBC_RO_ALL;
    zbc_zone_t *z, *zones=NULL;
    unsigned int nr_zones=0;
    
    ret=zbc_report_nr_zones(dev,lba,ro,&nr_zones);
    if(ret != 0)
    {
    	printf("Report Zone Number Failure!\n");
    	return -1;
    }
    printf("-+-nr_zones=%d\n",nr_zones);
    
    if(index > nr_zones)
    {
    	printf("Index Error!\n");
    	return -1;
    }
    
    zones=(zbc_zone_t *)malloc(sizeof(zbc_zone_t)*nr_zones);
    if (!zones) 
    {
		printf("malloc Error!\n");
		return -1;
    }
    memset(zones, 0, sizeof(zbc_zone_t)*nr_zones);
    
    ret=zbc_report_zones(dev,lba,ro,zones,&nr_zones);
    if(ret!=0)
    {
    	printf("Report Zone Failure!\n");
    	return -1;
    }
    
    z=&zones[index];
    if ( zbc_zone_conventional(z) ) {
            printf("Zone %05d: type 0x%x (%s), cond 0x%x (%s), LBA %llu, %llu sectors, wp N/A\n",
                   i,
                   zbc_zone_type(z),
                   zbc_zone_type_str(zbc_zone_type(z)),
                   zbc_zone_condition(z),
                   zbc_zone_condition_str(zbc_zone_condition(z)),
                   zbc_zone_start_lba(z),
                   zbc_zone_length(z));
        } else {
            printf("Zone %05d: type 0x%x (%s), cond 0x%x (%s), need_reset %d, non_seq %d, LBA %llu, %llu sectors, wp %llu\n",
                   i,
                   zbc_zone_type(z),
                   zbc_zone_type_str(zbc_zone_type(z)),
                   zbc_zone_condition(z),
                   zbc_zone_condition_str(zbc_zone_condition(z)),
                   zbc_zone_need_reset(z),
                   zbc_zone_non_seq(z),
                   zbc_zone_start_lba(z),
                   zbc_zone_length(z),
                   zbc_zone_wp_lba(z));
        }
        
        return 1;

}









