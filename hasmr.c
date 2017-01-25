#include <libzbc/zbc.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


struct smr_info{
    struct zbc_device *dev;
};

struct smr_info *smr_open(char *path,struct smr_info *smr);
int smr_close(struct smr_info *smr);
int smr_report(struct smr_info *smr,unsigned int index);

int main()
{
	int ret;
    struct smr_info *smr;
//	struct zbc_device *dev;
	char path[64]="/dev/sdb";
	
    smr=(struct smr_info *)malloc(sizeof(struct smr_info));

	smr=smr_open(path,smr);
    
    ret=smr_report(smr,1000);
	if(ret != 1)
	{
		printf("REPORT Failure..\n");
		exit(-1);
	}
    printf("outing smr_report()\n");	

	ret=smr_close(smr);
	if(ret != 1)
	{
		printf("CLOSE Failure..\n");
		exit(-1);
	}

	return 1;
}



struct smr_info *smr_open(char *path,struct smr_info *smr)
{
	int ret = 1;
	struct zbc_device_info info;
    enum zbc_reporting_options ro = ZBC_RO_ALL;
    
    ret=zbc_open(path,O_RDONLY,&smr->dev);
    if(ret != 0)
    {
    	printf("Open Device Failure!\n");
    	exit(-1);
    }
    
    ret=zbc_get_device_info(smr->dev,&info);
    if(ret < 0)
    {
    	printf("Get Device Info Failure!\n");
    	//return -1;
    	exit(-1);
    }
    
    printf("Device %s: %s\n",path,info.zbd_vendor_id);
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
		zbc_close(smr->dev);
        printf("CLOSE SUCCESS!\n");
		return 1;
	}
	printf("CLOSE Failure..\n");
	return -1;	
}


int smr_report(struct smr_info *smr,unsigned int index)
{
	int ret;
	unsigned long long lba=0;
    enum zbc_reporting_options ro=ZBC_RO_ALL;
    struct zbc_zone *z, *zones=NULL;
    int nr_zones=29000;
    
    printf("entering smr_report()\n");
    
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
    
    printf("1111--%d\n",ret);
    if(!smr->dev)
    {
        printf("8888\n");
    }
    ret=zbc_report_zones(smr->dev,lba,ro,zones,&nr_zones);
    printf("2222--%d\n",ret);
    if(ret!=0)
    {
    	printf("Report Zone Failure!\n");
    	return -1;
    }
    printf("3333\n");
    
    z=&zones[index];
    if ( zbc_zone_conventional(z) ) {
            printf("Zone %05d: type 0x%x (%s), cond 0x%x (%s), LBA %llu, %llu sectors, wp N/A\n",
                   index,
                   zbc_zone_type(z),
                   zbc_zone_type_str(zbc_zone_type(z)),
                   zbc_zone_condition(z),
                   zbc_zone_condition_str(zbc_zone_condition(z)),
                   zbc_zone_start_lba(z),
                   zbc_zone_length(z));
        } else {
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
        }
        
    if(zones)
    {
        free(zones);
    }
    return 1;

}









